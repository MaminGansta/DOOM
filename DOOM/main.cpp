#include <windows.h>

#include <cstdint>
#include <cassert>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>

// some globals
#define PI 3.14159265359f
#define MAX_BULLETS 20
#define MAX_ENEMIES 10
bool running = true;


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "render_stuff.cpp"
Render_State surface;

#include "image.cpp"
#include "timer.cpp"
#include "input.cpp"
#include "thread_pool.cpp"

#include "animation/animation.cpp"

#include "bullets/bullet.cpp"
#include "enemies/enemy.cpp"


LRESULT CALLBACK win_callback(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lParam)
{
	LRESULT res = 0;

	switch (uMsg)
	{
		case WM_CLOSE:
		case WM_DESTROY:
		{
			running = false;
		} break;

		case WM_SIZE:
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			surface.width = rect.right - rect.left;
			surface.height = rect.bottom - rect.top;

			int size = surface.width * surface.height * sizeof(unsigned int);

			if (surface.memory) VirtualFree(surface.memory, 0 , MEM_RELEASE);
			surface.memory = (Color*)VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			surface.bitmap_info.bmiHeader.biSize = sizeof(surface.bitmap_info.bmiHeader);
			surface.bitmap_info.bmiHeader.biWidth = surface.width;
			surface.bitmap_info.bmiHeader.biHeight = surface.height;
			surface.bitmap_info.bmiHeader.biPlanes = 1;
			surface.bitmap_info.bmiHeader.biBitCount = 32;
			surface.bitmap_info.bmiHeader.biCompression = BI_RGB;

		} break;

		default:
		{
			res = DefWindowProc(hwnd, uMsg, wparam, lParam);
		}
	}
	return res;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPiv, LPSTR args, int someshit)
{
	// create window class
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpszClassName = "DOOM";
	window_class.lpfnWndProc = win_callback;

	// reg window
	RegisterClass(&window_class);

	// create window
	HWND window = CreateWindow(window_class.lpszClassName, "DOOM", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 800, 620, 0, 0, hInst, 0);
	HDC hdc = GetDC(window);

	// GAME VARS------------------------------
	Color* column1 = new Color[2048];
	Color* column2 = new Color[2048];
	Color* column3 = new Color[2048];
	Color* column4 = new Color[2048]; // for each thread


	float* depth_buffer = new float[1980];

	// player position
	float player_x = 2.0f; // player x position
	float player_y = 2.0f; // player y position
	float player_a = 1.523; // player view direction
	float player_step = 0.0f; // one step distance

	// before calculation, for collision detection
	float new_player_x = player_x;
	float new_player_y = player_y;


	// speed
	float speed_limit = 5;
	float speed_change = 28;

	// speed x
	float speed_x = 0.0f;
	bool moving_x = false;
	int speed_x_dir = 1;

	// speed y
	float speed_y = 0.0f;
	bool moving_y = false;
	int speed_y_dir = 1;


	// gun shift animation
	float gun_shift = 0;
	int gun_shift_dir = 1;
	bool shot = false;

	// gun
	int gun_texture_id = 0;
	Animation pistol_anime(3, 0.07f);

	// player vars
	int player_hp = 100;
	bool get_damage = false;
	Animation damageAnime(1, 0.15f);
	Animation deathAnime(1, 0.4f, 1);

	// map
	const size_t map_w = 16; // map width
	const size_t map_h = 16; // map height
	const char map[] =
		"0000111111111000"
		"1              0"
		"1     111111   0"
		"1     0        0"
		"0     0  1110000"
		"0     1        0"
		"0   100        0"
		"0   0   11100  0"
		"0   0   0      0"
		"0   0   1  00000"
		"0       1      0"
		"1       1      0"
		"0       0      0"
		"0 0000000      0"
		"0              0"
		"0001111111100000"; // our game map
	assert(sizeof(map) == map_w * map_h + 1); // +1 for the null terminated string


	const size_t map_cell_w = surface.width / map_w / 5;
	const size_t map_cell_h = surface.height / map_h / 4;

	const size_t map_screen_w = surface.width / map_w / 5 * map_w;
	const size_t map_screen_h = surface.height / map_h / 4 * map_h;


	// walls colors on map
	Color* map_colors = new Color[10];
	for (size_t i = 0; i < 10; i++)
		map_colors[i] = Color((i * 130) % 255, (i * 32) % 255, rand() % 255);


	// load textures 
	// walls
	Sprites walltext("textures/walls2.png");
	if (walltext.invalid) return 1;

	// load background
	Sprites sky("textures/sky1.png");
	if (sky.invalid) return 1;

	// load gun sprites
	Sprites gun("textures/pistol.png");
	if (gun.invalid) return 1;

	// imp sprite
	Sprites imp_live("textures/imp_movement.png");
	if (imp_live.invalid) return 1;
	Sprites imp_death("textures/imp_hit_death.png");
	if (imp_death.invalid) return 1;


	// create enemies
	enemy_sprites[ENEMY_IMP].live_sprites = std::move(imp_live);
	enemy_sprites[ENEMY_IMP].death_sprites = std::move(imp_death);

	Enemy_buffer<MAX_ENEMIES> enemies;
	enemies.add(100, 3, 7);
	enemies.add(100, 2, 8);
	enemies.add(100, 10, 9);
	enemies.add(100, 14, 11);
	enemies.add(100, 6, 14.5f);
	enemies.add(100, 6, 8);
	enemies.add(100, 13, 3);
	enemies.add(100, 10, 6);
	enemies.add(100, 8, 4);


	// enemyes bullets
	Sprites bullet("textures/imp_bullet.png");
	if (bullet.invalid) return 1;

	bullet_spites[IMP_BULLET] = std::move(bullet);
	Bullet_buffer<MAX_BULLETS> bullets;
	
	// thread pool
	thread_pool workers(4);

	// input
	Input input;

	bool DEBUG = false;

	// GAME LOOP---------------------------------
	Timer timer;
	while (running)
	{
		// clear the depth_buffer
		for (int i = 0; i < 1980; i++)
			depth_buffer[i] = 1e3;

		// clear buttons flags
		for (int i = 0; i < BUTTON_COUNT; i++)
			input.buttons[i].changed = false;

		// clear shot flag
		shot = false;
	
		// Input
		MSG msg;
		while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
			case WM_KEYUP:
			case WM_KEYDOWN:
			{
				uint32_t vk_code = (uint32_t)msg.wParam;
				bool is_down = ((msg.lParam & (1 << 31)) == 0);

				switch (vk_code)
				{
				case VK_LEFT:
				{
					input.buttons[BUTTON_LROTATE].changed = true;
					input.buttons[BUTTON_LROTATE].is_down = is_down;
				}break;
				case VK_RIGHT:
				{
					input.buttons[BUTTON_RROTATE].changed = true;
					input.buttons[BUTTON_RROTATE].is_down = is_down;
				}break;
				case VK_W:
				{
					input.buttons[BUTTON_UP].changed = is_down;
					input.buttons[BUTTON_UP].is_down = is_down;
				}break;
				case VK_S:
				{
					input.buttons[BUTTON_DOWN].changed = true;
					input.buttons[BUTTON_DOWN].is_down = is_down;
				}break;
				case VK_A:
				{
					input.buttons[BUTTON_LEFT].changed = true;
					input.buttons[BUTTON_LEFT].is_down = is_down;
				}break;
				case VK_D:
				{
					input.buttons[BUTTON_RIGHT].changed = true;
					input.buttons[BUTTON_RIGHT].is_down = is_down;
				}break;
				case VK_SPACE:
				{
					input.buttons[BUTTON_SHOT].changed = is_down != input.buttons[BUTTON_SHOT].is_down;
					input.buttons[BUTTON_SHOT].is_down = is_down;
				}break;
				case VK_DEBUG:
				{
					DEBUG = true;
				}break;
				case VK_ESCAPE:
				{
					running = false;
				}break;
			}
			}break;
			default:
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}break;
			}
		}

		if (surface.height == 0 || surface.width == 0)
			continue;


		// Break Point here
		if (DEBUG)
			DEBUG = false;

		// if shooting
		if (input.buttons[BUTTON_SHOT].is_down & input.buttons[BUTTON_SHOT].changed)
			shot = true;

		// Movement
		if (input.buttons[BUTTON_RROTATE].is_down)
			player_a += 2.2f * timer.elapsed;

		if (input.buttons[BUTTON_LROTATE].is_down)
			player_a -= 2.2f * timer.elapsed;

		while (player_a > 2*PI) player_a -= 2 * PI;
		while (player_a < 0) player_a += 2 * PI;


		if (input.buttons[BUTTON_UP].is_down)
		{
			moving_x = true;
			speed_x_dir = 1;
		}
		else if (input.buttons[BUTTON_DOWN].is_down)
		{
			moving_x = true;
			speed_x_dir = (-1);
		}
		else
			moving_x = false;
		
		
		if (input.buttons[BUTTON_LEFT].is_down)
		{
			moving_y = true;
			speed_y_dir = -1;
		}
		else if (input.buttons[BUTTON_RIGHT].is_down)
		{
			moving_y = true;			
			speed_y_dir = 1;
		}
		else
			moving_y = false;


		if (moving_x)
		{
			if (speed_x_dir > 0)
				speed_x = speed_x > speed_limit ? speed_limit : speed_x + speed_change * timer.elapsed;
			else
				speed_x = speed_x < (-1) * speed_limit ? (-1) * speed_limit : speed_x - speed_change * timer.elapsed;
		}
		else
		{
			if (speed_x > 0)
				speed_x = speed_x < 1 ? 0.0f : speed_x - speed_change * timer.elapsed;
			else
				speed_x = speed_x > -1 ? 0.0f : speed_x + speed_change * timer.elapsed;
		}

		if (moving_y)
		{
			if (speed_y_dir > 0)
				speed_y = speed_y > speed_limit ? speed_limit : speed_y + speed_change * timer.elapsed;
			else
				speed_y = speed_y < (-1) * speed_limit ? (-1) * speed_limit : speed_y - speed_change * timer.elapsed;
		}
		else
		{
			if (speed_y > 0)
				speed_y = speed_y < 1 ? 0.0f : speed_y - speed_change * timer.elapsed;
			else
				speed_y = speed_y > -1 ? 0.0f : speed_y + speed_change * timer.elapsed;
		}

		// x direction
		new_player_x += timer.elapsed * cosf(player_a)  * speed_x;
		new_player_y += timer.elapsed * sinf(player_a)  * speed_x;

		// y direction
		new_player_x += timer.elapsed * cosf(player_a + PI / 2) * speed_y;
		new_player_y += timer.elapsed * sinf(player_a + PI / 2) * speed_y;

		
		// Collision detection
		if ((new_player_x > 0 && new_player_x < map_cell_w * map_cell_h && new_player_y > 0 && new_player_y < map_cell_w * map_cell_w) &&
			(map[(int)(new_player_y + 0.25f) * map_w + (int)(new_player_x + 0.25f)] != ' ' ||
			 map[(int)(new_player_y - 0.25f) * map_w + (int)(new_player_x - 0.25f)] != ' ' ||
			 map[(int)(new_player_y + 0.25f) * map_w + (int)(new_player_x - 0.25f)] != ' ' ||
			 map[(int)(new_player_y - 0.25f) * map_w + (int)(new_player_x + 0.25f)] != ' '))
		{
			// if wall
			float x_dif = new_player_x - player_x;
			float y_dif = new_player_y - player_y;

			// horizontal wall collision
			if (map[(int)(player_y + 0.25f + y_dif) * map_w + (int)(player_x + 0.25f)] == ' ' &&
				map[(int)(player_y - 0.25f + y_dif) * map_w + (int)(player_x - 0.25f)] == ' ')
			{
				player_step = fabs(y_dif);
				player_y = new_player_y = player_y + y_dif; // slide on the horizontal wall
				new_player_x = player_x;

			}
			// vertical wall collision
			else if (map[(int)(player_y + 0.25f) * map_w + (int)(player_x + 0.25f + x_dif)] == ' ' &&
				     map[(int)(player_y - 0.25f) * map_w + (int)(player_x - 0.25f + x_dif)] == ' ')
			{
				player_step = fabs(x_dif);
				new_player_y = player_y;
				player_x = new_player_x = player_x + x_dif; // slide on the vertical wall
			}
			else
			{
				player_step = 0; //fabs(fabs(new_player_y) - fabs(player_y)) + fabs(fabs(new_player_x) - fabs(player_x));
				new_player_y = player_y;
				new_player_x = player_x;
			}
		}
		else // no wall on the way
		{
			player_step = fabs(fabs(new_player_y) - fabs(player_y)) + fabs(fabs(new_player_x) - fabs(player_x));
			player_y = new_player_y;
			player_x = new_player_x;
		}


		// draw the background
		// floor
		auto _floor = workers.add_task([]() {
			draw_rectangle(0, 0, surface.width, surface.height / 2, Color(130, 130, 130));
		});
		// sky
		auto _skys = workers.add_task([&sky]() {
			for (int y = surface.height / 2; y < surface.height; y++)
			{
				for (int x = 0; x < surface.width; x++)
				{
					Color color = sky.data[(int)(y * ((float)sky.width / surface.height / 2)) * sky.width + (int)(x * ((float)sky.width / surface.width))];
					surface.memory[y * surface.width + x] = color;
				}
			}
		});
		
		// wait 
		_floor.get();
		_skys.get();


		// Walls ---------------------------------------------------------------------------------
		const float fov = PI / 3.0f; // field of view

		// ray caster
		auto wall1 = workers.add_task([&]() {
			for (int i = 0; i < surface.width / 4; i++)
			{
				float angle = player_a - fov / 2 + fov * i / float(surface.width);


				// ray casting
				for (float t = 0.0f; t < 20; t += 0.025f)
				{
					float cx = player_x + t * cos(angle);
					float cy = player_y + t * sin(angle);

					// ray on map
					size_t pix_x = cy * map_cell_w;
					size_t pix_y = cx * map_cell_h;
					surface.memory[pix_x + pix_y * surface.width] = Color(240, 240, 240);

					// walls
					if (map[int(cx) + int(cy) * map_w] != ' ')
					{
						size_t column_height = min(2048, (surface.height / t / cos(angle - player_a)));
						int texid = (int)map[int(cx) + int(cy) * map_w] - 48;

						float hitx = cx - floor(cx + 0.5f);
						float hity = cy - floor(cy + 0.5f);
						int x_texcoord = hitx * walltext.width;
						if (std::abs(hity) > std::abs(hitx))
						{
							x_texcoord = hity * walltext.width;
						}
						if (x_texcoord < 0) x_texcoord += walltext.width;
						assert(x_texcoord >= 0 && x_texcoord < (int)walltext.width);


						texture_column(column1, walltext, texid, x_texcoord, column_height);
						pix_x = i;
						for (size_t j = 0; j < column_height; j++) {
							pix_y = j + surface.height / 2 - column_height / 2;
							if (pix_y < 0 || pix_y >= surface.height) continue;
							if (pix_x < map_screen_w && pix_y < map_screen_h) continue;
							surface.memory[pix_x + pix_y * surface.width] = column1[j];
							// depth_buffer
							depth_buffer[pix_x] = t;
						}
						break;
					}
				}
			}
		});

		// ray caster
		auto wall2 = workers.add_task([&]() {
			for (int i = surface.width / 4; i < surface.width / 2; i++)
			{
				float angle = player_a - fov / 2 + fov * i / float(surface.width);


				// ray casting
				for (float t = 0.0f; t < 20; t += 0.025f)
				{
					float cx = player_x + t * cos(angle);
					float cy = player_y + t * sin(angle);

					// ray on map
					size_t pix_x = cy * map_cell_w;
					size_t pix_y = cx * map_cell_h;
					surface.memory[pix_x + pix_y * surface.width] = Color(240, 240, 240);

					// walls
					if (map[int(cx) + int(cy) * map_w] != ' ')
					{
						size_t column_height = min(2048, (surface.height / t / cos(angle - player_a)));
						int texid = (int)map[int(cx) + int(cy) * map_w] - 48;

						float hitx = cx - floor(cx + 0.5f);
						float hity = cy - floor(cy + 0.5f);
						int x_texcoord = hitx * walltext.width;
						if (std::abs(hity) > std::abs(hitx))
						{
							x_texcoord = hity * walltext.width;
						}
						if (x_texcoord < 0) x_texcoord += walltext.width;
						assert(x_texcoord >= 0 && x_texcoord < (int)walltext.width);


						texture_column(column2, walltext, texid, x_texcoord, column_height);
						pix_x = i;
						for (size_t j = 0; j < column_height; j++) {
							pix_y = j + surface.height / 2 - column_height / 2;
							if (pix_y < 0 || pix_y >= surface.height) continue;
							surface.memory[pix_x + pix_y * surface.width] = column2[j];
							// depth_buffer
							depth_buffer[pix_x] = t;
						}
						break;
					}
				}
			}
		});

		auto wall3 = workers.add_task([&]() {
			for (int i = surface.width / 2; i < 3 * surface.width / 4; i++)
			{
				float angle = player_a - fov / 2 + fov * i / float(surface.width);


				// ray casting
				for (float t = 0.0f; t < 20; t += 0.025f)
				{
					float cx = player_x + t * cos(angle);
					float cy = player_y + t * sin(angle);

					// ray on map
					size_t pix_x = cy * map_cell_w;
					size_t pix_y = cx * map_cell_h;
					surface.memory[pix_x + pix_y * surface.width] = Color(240, 240, 240);

					// walls
					if (map[int(cx) + int(cy) * map_w] != ' ')
					{
						size_t column_height = min(2048, (surface.height / t / cos(angle - player_a)));
						int texid = (int)map[int(cx) + int(cy) * map_w] - 48;

						float hitx = cx - floor(cx + 0.5f);
						float hity = cy - floor(cy + 0.5f);
						int x_texcoord = hitx * walltext.width;
						if (std::abs(hity) > std::abs(hitx))
						{
							x_texcoord = hity * walltext.width;
						}
						if (x_texcoord < 0) x_texcoord += walltext.width;
						assert(x_texcoord >= 0 && x_texcoord < (int)walltext.width);


						texture_column(column3, walltext, texid, x_texcoord, column_height);
						pix_x = i;
						for (size_t j = 0; j < column_height; j++) {
							pix_y = j + surface.height / 2 - column_height / 2;
							if (pix_y < 0 || pix_y >= surface.height) continue;
							surface.memory[pix_x + pix_y * surface.width] = column3[j];
							// depth_buffer
							depth_buffer[pix_x] = t;
						}
						break;
					}
				}
			}
		});

		// ray caster
		auto wall4 = workers.add_task([&]() {
			for (int i = 3 * surface.width / 4; i < surface.width; i++)
			{
				float angle = player_a - fov / 2 + fov * i / float(surface.width);


				// ray casting
				for (float t = 0.0f; t < 20; t += 0.025f)
				{
					float cx = player_x + t * cos(angle);
					float cy = player_y + t * sin(angle);

					// ray on map
					size_t pix_x = cy * map_cell_w;
					size_t pix_y = cx * map_cell_h;
					surface.memory[pix_x + pix_y * surface.width] = Color(240, 240, 240);

					// walls
					if (map[int(cx) + int(cy) * map_w] != ' ')
					{
						size_t column_height = min(2048, (surface.height / t / cos(angle - player_a)));
						int texid = (int)map[int(cx) + int(cy) * map_w] - 48;

						float hitx = cx - floor(cx + 0.5f);
						float hity = cy - floor(cy + 0.5f);
						int x_texcoord = hitx * walltext.width;
						if (std::abs(hity) > std::abs(hitx))
						{
							x_texcoord = hity * walltext.width;
						}
						if (x_texcoord < 0) x_texcoord += walltext.width;
						assert(x_texcoord >= 0 && x_texcoord < (int)walltext.width);


						texture_column(column4, walltext, texid, x_texcoord, column_height);
						pix_x = i;
						for (size_t j = 0; j < column_height; j++) {
							pix_y = j + surface.height / 2 - column_height / 2;
							if (pix_y < 0 || pix_y >= surface.height) continue;
							surface.memory[pix_x + pix_y * surface.width] = column4[j];
							// depth_buffer
							depth_buffer[pix_x] = t;
						}
						break;
					}
				}
			}
		});

		wall1.get();
		wall2.get();
		wall3.get();
		wall4.get();

		// not the best solution but i'm too lazzy
		// 2x times faster then single thread

		// draw map
		for (int i = 0; i < map_h; i++)
		{
			for (int j = 0; j < map_w; j++)
			{
				int x = j * map_cell_w;
				int y = i * map_cell_h;

				if (map[i * map_w + j] != ' ')
					draw_rectangle(y, x, map_cell_w, map_cell_h, map_colors[map[i * map_w + j] - 48]); // -48 get id from char
			}
		}

		// draw player on map
		draw_rectangle(player_y * map_cell_w - 2, player_x * map_cell_h - 2, 5, 5, Color(60, 60, 60));


		// enemies --------------------------------------------------------------------------------------------
		for (int n = 0; n < enemies.actives; n++)
		{
			int text_id = 0;
			enemies[n].visible = false;

			// absolute direction from the player to the sprite (in radians)
			float sprite_dir = atan2(enemies[n].pos_y - player_y, enemies[n].pos_x - player_x);


			// death sprite animation
			if (enemies[n].hit && enemies[n].hp > 0)
			{
				if (enemies[n].hit_animation.cycles)
					text_id = enemies[n].hit_animation.sprite(timer.elapsed);
				else
					enemies[n].hit = false;
			}
			else if (enemies[n].hp <= 0)
			{
				int frame = enemies[n].death_animation.sprite(timer.elapsed);
				frame = frame || enemies[n].death_animation.cycles != 0 ? frame + 8 : frame + 12;
				text_id = frame;
			}

			// remove unnecessary periods from the relative direction
			while (sprite_dir - player_a > PI) sprite_dir -= 2 * PI;
			while (sprite_dir - player_a < -PI) sprite_dir += 2 * PI;

			// distance from the player to the sprite
			enemies[n].distance = sqrt(pow(player_x - enemies[n].pos_x, 2) + pow(player_y - enemies[n].pos_y, 2));
			size_t sprite_screen_size = min(2000, static_cast<int>(surface.height / enemies[n].distance));

			int h_offset = (sprite_dir - player_a) * (surface.width) / (fov) + (surface.width) / 2 - sprite_screen_size / 2;
			int v_offset = surface.height / 2 - sprite_screen_size / 2;

			// enemy on map
			if (fabs(sprite_dir - player_a) < fov / 2 && enemies[n].distance < 20 && depth_buffer[h_offset] > enemies[n].distance && enemies[n].hp > 0)
				draw_rectangle(enemies[n].pos_y * map_cell_w - 2, enemies[n].pos_x * map_cell_h - 2, 3, 3, Color(240, 10, 10));

			for (size_t i = 0; i < sprite_screen_size; i++)
			{
				if (h_offset + int(i) < 0 || h_offset + i >= surface.width) continue;
				
				if (depth_buffer[h_offset + int(i)] > enemies[n].distance)
				{
					if (enemies[n].hp > 0)
						depth_buffer[h_offset + int(i)] = enemies[n].distance;
				}
				else continue;

				for (size_t j = 1; j < sprite_screen_size; j++)
				{
					if (v_offset + int(j) < 0 || v_offset + j >= surface.height) continue;
					
					enemies[n].visible = true;

					bool sprite_type = !(enemies[n].hp > 0 && !enemies[n].hit);
					int w = enemy_sprites[ENEMY_IMP][sprite_type].width;
					int count = enemy_sprites[ENEMY_IMP][sprite_type].count;
					Color color = enemy_sprites[ENEMY_IMP][sprite_type].data[(int)(i * (float)w / sprite_screen_size) + (int)((sprite_screen_size - j) * (float)w / sprite_screen_size) * count * w + w * text_id];


					// filter the background
					if (color.r < 140 && color.b > 60 && color.g > 55) continue;
					surface.memory[h_offset + i + (v_offset + j) * surface.width] = color;
				}
			}

			// behavior
			if (enemies[n].hp > 0)
				enemies[n].behavior(enemies[n], player_x, player_y, timer.elapsed, bullets);
		}

		// sort enemies
		std::sort(enemies.begin(), enemies.end(), [](Enemy& a, Enemy& b) { return a.distance > b.distance; });



		// gun  animation -----------------------------------------------------------------------------------------
		if (shot)
			pistol_anime.add_cycle(1);
		
		gun_texture_id = pistol_anime.sprite(timer.elapsed);

		// draw gun
		if (gun_shift < 0)
			gun_shift_dir = 1;
		if (gun_shift > 10)
			gun_shift_dir = -1;
		if (player_step > 1e-2)
		{
			gun_shift += gun_shift_dir * timer.elapsed * 30;
		}

		int gun_h = int(surface.height / 2.3f);
		int gun_w = surface.width / 3;

		for (int i = 0; i < gun_h; i++)
		{
			for (int j = 0; j < gun_w; j++)
			{
				Color color = gun.data[(int)((gun_h - i) * ((float)gun.width / gun_h)) * gun.width * gun.count + (int)(j * ((float)gun.width / gun_w)) + gun.width * gun_texture_id];

				// filter the image background
				if (color.r < 80 && color.b > 90 && color.g > 85) continue;

				if ((i + (int)gun_shift - 20) * surface.width + j + (int)(surface.width / 2.9f) < 0) continue;

				surface.memory[(i + (int)gun_shift - 20) * surface.width + j + (int)(surface.width / 2.9f)] = color;
			}
		}

		// shot handler
		if (shot)
		{
			for (int i = 0; i < enemies.actives; i++)
			{
				// absolute direction from the player to the sprite (in radians)
				float sprite_dir = atan2(enemies[i].pos_y - player_y, enemies[i].pos_x - player_x);

				// remove unnecessary periods from the relative direction
				while (sprite_dir - player_a > PI) sprite_dir -= 2 * PI;
				while (sprite_dir - player_a < -PI) sprite_dir += 2 * PI;

				if (fabs(fabs(player_a) - fabs(sprite_dir - 0.07f)) < 0.07f && enemies[i].visible)
				{
					enemies[i].hp -= 50;
					enemies[i].hit = true;
					enemies[i].hit_animation.add_cycle(1);
				}
			}
		}

		// sort enemyes bullets by distance
		if (bullets.actives > 1)
			std::sort(bullets.begin(), bullets.end(), [](Bullet& a, Bullet& b) { return a.distance > b.distance; });


		// bullets ---------------------------------------------------------------------------------------------------
		for (int n = 0; n < bullets.actives; n++)
		{
			// BULLET HANDLER-------------------

			// bullet movment
			bullets[n].pos_x += timer.elapsed * cosf(bullets[n].angle) * bullets[n].speed;
			bullets[n].pos_y += timer.elapsed * sinf(bullets[n].angle) * bullets[n].speed;

			// if in wall
			if (map[int(bullets[n].pos_y) * map_w + int(bullets[n].pos_x)] != ' ')
			{
				bullets.remove(n--);
				continue;
			}

			// if player
			if (fabs(bullets[n].pos_y - player_y) < 0.2f && fabs(bullets[n].pos_x - player_x) < 0.2f)
			{
				player_hp -= 20;
				get_damage = true;
				damageAnime.add_cycle(1);
				bullets.remove(n--);
				continue;
			}

			//-----------------------------------

			// DRAW BULLET

			// angle of bullet
			float sprite_dir = atan2(bullets[n].pos_y - player_y, bullets[n].pos_x - player_x);

			while (sprite_dir - player_a > PI) sprite_dir -= 2 * PI;
			while (sprite_dir - player_a < -PI) sprite_dir += 2 * PI;

			// distance from the player to the bullet
			bullets[n].distance = sqrt(pow(player_x - bullets[n].pos_x, 2) + pow(player_y - bullets[n].pos_y, 2));
			size_t sprite_screen_size = min(2000, static_cast<int>(surface.height / bullets[n].distance));

			int h_offset = (sprite_dir - player_a) * (surface.width) / (fov)+(surface.width) / 2 - sprite_screen_size / 2;
			int v_offset = surface.height / 2 - sprite_screen_size / 2;

			// bullet on map
			if (fabs(sprite_dir - player_a) < fov / 2 && bullets[n].distance < 20 && depth_buffer[h_offset] > bullets[n].distance)
				draw_rectangle(bullets[n].pos_y * map_cell_w - 2, bullets[n].pos_x * map_cell_h - 2, 2, 2, Color(240, 10, 10));

			int bw = bullet_spites[IMP_BULLET].width;
			int bcount = bullet_spites[IMP_BULLET].count;


			for (size_t i = 0; i < sprite_screen_size; i++)
			{
				if (h_offset + int(i) < 0 || h_offset + i >= surface.width) continue;

				if (depth_buffer[h_offset + int(i)] > bullets[n].distance)
				{
					depth_buffer[h_offset + int(i)] = bullets[n].distance;
				}
				else continue;

				for (size_t j = 1; j < sprite_screen_size; j++)
				{
					if (v_offset + int(j) < 0 || v_offset + j >= surface.height) continue;

					// take a pixel of texture (relative by screen size)
					Color color = bullet_spites[IMP_BULLET].data[(int)(i * (float)bw / sprite_screen_size) + (int)((sprite_screen_size - j) * (float)bw / sprite_screen_size) * bcount * bw];

					// filter the background
					if (color.r == 255 && color.b == 0 && color.g == 110) continue;
					surface.memory[h_offset + i + (v_offset + j) * surface.width] = color;
				}
			}
		}

		// damage animation
		if (damageAnime.cycles > 0)
		{
			damageAnime.sprite(timer.elapsed);

			for (int i = 0; i < surface.width * surface.height; i++)
				surface.memory[i].r = 255; // red screen
		}

		// death animation
		if (player_hp < 1)
		{
			deathAnime.sprite(timer.elapsed);

			if (deathAnime.cycles == 0)
				return 0;

			for (int i = 0; i < surface.width * surface.height; i++)
				surface.memory[i] = Color(255, 0, 0); // red screen
		}

		// Render
		StretchDIBits(hdc, 0, 0, surface.width, surface.height, 0, 0, surface.width, surface.height, surface.memory, &surface.bitmap_info, DIB_RGB_COLORS, SRCCOPY);

		// Log
		char info[32];
		sprintf_s(info, "fps_%d  frame time_%.4f sec\n", timer.FPS, timer.elapsed);
		OutputDebugString(info);

		// Timer
		timer.update();

	}
		
	//dump_log();
	delete[] depth_buffer;
	delete[] column1;
	delete[] column2;
	delete[] column3;

	return 0;
}