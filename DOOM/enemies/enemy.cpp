struct Enemy
{
	int m_hp;
	float m_pos_x;
	float m_pos_y;
	float m_agle;
	float m_distance{1};
	bool visible;
	Animation death_animation{ 5, 0.1f, 1 };
	
	bool hit{ false };
	Animation hit_animation{ 1, 0.2f, 0 };

	float attack_deley{ 0.6f };
	float attack_deley_time{ 0 };

	static void (*behavior)(Enemy* imp, float pl_x, float pl_y, float frame_time, std::vector<Bullet*>& bullets);

	static uint32_t* death;
	static uint32_t* sprites;

	inline Enemy() = default;

	inline Enemy(int hp, float x, float y, float angle) : m_hp(hp), m_pos_x(x), m_pos_y(y), m_agle(angle){}

};

void (*Enemy::behavior)(Enemy* imp, float pl_x, float pl_y, float frame_time, std::vector<Bullet*>& bullets) = NULL;
uint32_t* Enemy::sprites = NULL;
uint32_t* Enemy::death = NULL;

