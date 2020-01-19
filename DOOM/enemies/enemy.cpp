

enum Enemis_id { ENEMY_IMP,  ENEMY_COUNT };

struct Enemy_sprites
{
	// look up table
	union
	{
		struct
		{
			Sprites live_sprites;
			Sprites death_sprites;
		};
		Sprites type[2];
	};

	Enemy_sprites() : live_sprites(), death_sprites() {}
	~Enemy_sprites()
	{
		live_sprites.~Sprites();
		death_sprites.~Sprites();
	}

	Sprites& operator [] (int indx) { return type[indx]; }
};

// TODO make it local
Enemy_sprites enemy_sprites[ENEMY_COUNT];


struct Enemy
{
	int hp;
	float pos_x;
	float pos_y;
	float distance = 10;

	Animation death_animation{ 5, 0.1f, 1 };
	Animation hit_animation{ 1, 0.2f, 0 };

	float attack_deley = 0.6f;
	float attack_deley_time = 0.0f;

	uint8_t death_sprites_id;
	uint8_t live_sprites_id;

	bool visible;
	bool hit;

	// behavior for each enemy can be unique i think
	void (*behavior)(Enemy& imp, float pl_x, float pl_y, float frame_time, Bullet_buffer<MAX_BULLETS>& bullets);

	Enemy() : hp(100), pos_x(0), pos_y(0), visible(false), hit(false) {}
	Enemy(int hp, float x, float y) : hp(hp), pos_x(x), pos_y(y), visible(false), hit(false) {}
};

// enemy behavior
void imp_behavior(Enemy& imp, float pl_x, float pl_y, float frame_time, Bullet_buffer<MAX_BULLETS>& bullets)
{
	if (!imp.visible) return;

	imp.attack_deley_time -= frame_time;
	if (imp.attack_deley_time > 0) return;

	imp.attack_deley_time = imp.attack_deley;
	float bullet_angle = atan2(imp.pos_y - pl_y, imp.pos_x - pl_x);
	bullets.add(imp.pos_x, imp.pos_y, bullet_angle + PI, 8);
}

// enemy buffer
template <size_t max_size>
struct Enemy_buffer
{
	size_t actives;
	Enemy buffer[max_size];

	Enemy_buffer() : actives(0)
	{
		for (int i = 0; i < max_size; i++)
			buffer[i].behavior = imp_behavior;
	}

	void add(int hp, float pos_x, float pos_y)
	{
		if (actives == max_size) return;
		buffer[actives].hp = hp;
		buffer[actives].pos_x = pos_x;
		buffer[actives++].pos_y = pos_y;
	}

	void remove(size_t indx)
	{
		if (actives == 0 || indx > actives || indx < 0) return;
		using std::swap;
		swap(buffer[indx], buffer[--actives]);
	}

	Enemy& operator [] (size_t indx) { return buffer[indx]; }

	Enemy* begin() { return buffer; }
	Enemy* end() { return buffer + actives; }

};