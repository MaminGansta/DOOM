#include <windows.h>
#include <cstdint>
#include <cassert>

#include "input.h"
#include "timer.h"
#include "image.h"
#include "Log/log.h"
#include "render_stuff.h"


using namespace m::Timer;
#define PI 3.14159265359f

bool running = true;
Render_State surface;


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
			surface.memory = (uint32_t*)VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

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
	window_class.lpszClassName = "Game";
	window_class.lpfnWndProc = win_callback;

	// reg window
	RegisterClass(&window_class);

	// create window
	HWND window = CreateWindow(window_class.lpszClassName, "Game!!!", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 800, 620, 0, 0, hInst, 0);
	HDC hdc = GetDC(window);

	// GAME VARS------------------------------
	uint32_t column[2048];

	float player_x = 2.499; // player x position
	float player_y = 2.645; // player y position
	float player_a = 1.523; // player view direction


	// speed
	float speed_limit = 0.0000045f;
	float speed_change = 0.000000000015f;
	int speed_y_dir = 1;

	// speed x
	float speed_angle_x = 0.0f;
	float speed_x = 0.0f;
	bool moving_x = false;
	int speed_x_dir = 1;

	// speed y
	float speed_angle_y = 0.0f;
	float speed_y = 0.0f;
	bool moving_y = false;




	// map
	const size_t map_w = 16; // map width
	const size_t map_h = 16; // map height
	const char map[] =
		"0000111111111000"
		"1              0"
		"1     111111   0"
		"1     0        0"
		"0   1 0  1110000"
		"0   1 1        0"
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


	const size_t map_cell_w = win_w / map_w / 6;
	const size_t map_cell_h = win_h / map_h / 5;


	// colors
	const size_t ncolors = 10;
	uint32_t* colors = new uint32_t[ncolors];
	for (size_t i = 0; i < ncolors; i++) {
		colors[i] = pack_color((i * 130) % 255, (i * 32)% 255, rand() % 255);
	}


	// load textures
	uint32_t* walltext = NULL; // textures for the walls
	size_t walltext_size;  // texture dimensions (it is a square)
	size_t walltext_cnt;   // number of different textures in the image
	if (!load_texture("walls2.png", walltext, walltext_size, walltext_cnt))
	{
		return -1;
	}

	// input
	Input input;

	bool DEBUG = false;


	// GAME LOOP---------------------------------

	timer_init();
	while (running)
	{

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
					input.buttons[BUTTON_LROTATE].is_down = is_down;
					input.buttons[BUTTON_LROTATE].changed = true;

				}break;
				case VK_RIGHT:
				{
					input.buttons[BUTTON_RROTATE].is_down = is_down;
					input.buttons[BUTTON_RROTATE].changed = true;

				}break;
				case VK_W:
				{
					input.buttons[BUTTON_UP].is_down = is_down;
					input.buttons[BUTTON_UP].changed = true;

				}break;
				case VK_S:
				{
					input.buttons[BUTTON_DOWN].is_down = is_down;
					input.buttons[BUTTON_DOWN].changed = true;

				}break;
				case VK_A:
				{
					input.buttons[BUTTON_LEFT].is_down = is_down;
					input.buttons[BUTTON_LEFT].changed = true;

				}break;
				case VK_D:
				{
					input.buttons[BUTTON_RIGHT].is_down = is_down;
					input.buttons[BUTTON_RIGHT].changed = true;

				}break;
				case VK_DEBUG:
				{
					DEBUG = true;
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


		// Break Point
		if (DEBUG)
		{
			DEBUG = false;
		}

		// Movement
		if (input.buttons[BUTTON_RROTATE].is_down)
			player_a += 0.0000029f * nFrameTime;

		if (input.buttons[BUTTON_LROTATE].is_down)
			player_a -= 0.0000029f * nFrameTime;

		player_a = fmod(player_a, 2 * PI);



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
				speed_x = speed_x > speed_limit ? speed_limit : speed_x + speed_change * nFrameTime;
			else
				speed_x = speed_x < (-1) * speed_limit ? (-1) * speed_limit : speed_x - speed_change * nFrameTime;
		}
		else
		{
			if (speed_x > 0)
				speed_x = speed_x < 0.0000001f ? 0.0f : speed_x - speed_change * nFrameTime;
			else
				speed_x = speed_x > -0.0000001f ? 0.0f : speed_x + speed_change * nFrameTime;
		}

		if (moving_y)
		{
			if (speed_y_dir > 0)
				speed_y = speed_y > speed_limit ? speed_limit : speed_y + speed_change * nFrameTime;
			else
				speed_y = speed_y < (-1) * speed_limit ? (-1) * speed_limit : speed_y - speed_change * nFrameTime;
		}
		else
		{
			if (speed_y > 0)
				speed_y = speed_y < 0.0000001f ? 0.0f : speed_y - speed_change * nFrameTime;
			else
				speed_y = speed_y > -0.0000001f ? 0.0f : speed_y + speed_change * nFrameTime;
		}

		player_x += nFrameTime * cosf(player_a)  * speed_x;
		player_y += nFrameTime * sinf(player_a)  * speed_x;

		// Collision detection
		if (map[(int)(player_y + 0.15f) * map_w + (int)(player_x + 0.15f)] != ' ' ||
			map[(int)(player_y - 0.15f) * map_w + (int)(player_x - 0.15f)] != ' ')
		{
			player_x -= nFrameTime * cosf(player_a) * speed_x;
			player_y -= nFrameTime * sinf(player_a) * speed_x;
		}

		player_x += nFrameTime * cosf(player_a + PI / 2) * speed_y;
		player_y += nFrameTime * sinf(player_a + PI / 2) * speed_y;

		// Collision detection
		if (map[(int)(player_y + 0.15f) * map_w + (int)(player_x + 0.15f)] != ' ' ||
			map[(int)(player_y - 0.15f) * map_w + (int)(player_x - 0.15f)] != ' ')
		{
			player_x -= nFrameTime * cosf(player_a + PI / 2) * speed_y;
			player_y -= nFrameTime * sinf(player_a + PI / 2) * speed_y;
		}


		// Simulate
		const float fov = PI / 3.0f; // field of view

		// ray caster
		for (int i = 0; i < win_w ; i++)
		{
			float angle = player_a - fov / 2 + fov * i / float(surface.width);

			// clear screen
			if (i == 0)
			{
				draw_rectangle(&surface, 0,  0,         win_w, win_h / 2, pack_color(170, 170, 170));
				draw_rectangle(&surface, 0,  win_h / 2, win_w, win_h / 2, pack_color(255, 255, 255));
			}


			for (float t = 0.0f; t < 20; t += 0.025f)
			{
				float cx = player_x + t * cos(angle);
				float cy = player_y + t * sin(angle);

				size_t pix_x = cx * map_cell_w;
				size_t pix_y = cy * map_cell_h;
				surface.memory[pix_x + pix_y * surface.width] = pack_color(240, 240, 240);

				// ray casting
				if (map[int(cx) + int(cy) * map_w] != ' ')
				{
					size_t column_height = min(2048, (win_h / t / cos(angle - player_a)));
					int texid = (int)map[int(cx) + int(cy) * map_w] - 48;


					float hitx = cx - floor(cx + 0.5f); 
					float hity = cy - floor(cy + 0.5f); 
					int x_texcoord = hitx * walltext_size;
					if (std::abs(hity) > std::abs(hitx))
					{
						x_texcoord = hity * walltext_size;
					}
					if (x_texcoord < 0) x_texcoord += walltext_size; 
					assert(x_texcoord >= 0 && x_texcoord < (int)walltext_size);

					
					texture_column(column, walltext, walltext_size, walltext_cnt, texid, x_texcoord, column_height);
					pix_x = i;
					for (size_t j = 0; j < column_height; j++) {
						pix_y = j + win_h / 2 - column_height / 2;
						if (pix_y < 0 || pix_y >= win_h) continue;
						surface.memory[pix_x + pix_y * win_w] = column[j];
					}
					break;
				}
			}
		}

		// draw map
		for (int i = 0; i < map_h; i++)
		{
			for (int j = 0; j < map_w; j++)
			{
				int x = j * map_cell_w;
				int y = i * map_cell_h;

				if (map[i * map_w + j] != ' ')
					draw_rectangle(&surface, x, y, map_cell_w, map_cell_h, colors[map[i * map_w + j] - 48]); // -48 get id from char
				//else
					//draw_rectangle(&surface, x, y, map_cell_w, map_cell_h, pack_color(255, 255, 255));
			}
		}

		// draw player on map
		draw_rectangle(&surface, player_x * map_cell_w - 2, player_y * map_cell_h - 2, 5, 5, pack_color(60, 60, 60));


		


		// Render
		StretchDIBits(hdc, 0, 0, surface.width, surface.height, 0, 0, surface.width, surface.height, surface.memory, &surface.bitmap_info, DIB_RGB_COLORS, SRCCOPY);

		// Log
		//add_log(std::to_string(nFPS));

		// Timer
		timer_update();

	}
		
	dump_log();
	return 0;
}