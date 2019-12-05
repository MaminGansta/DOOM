struct Animation
{
	int m_nSprites_cnt;
	float m_Sprite_time;
	int cycles;

	float time;
	int cur_sprite{0};


	inline Animation(int cnt, float time, int cycles = 0): m_nSprites_cnt(cnt), m_Sprite_time(time), time(time), cycles(cycles){}

	inline int sprite(float frame_time)
	{
		if (cycles == 0)
			return cur_sprite;

		time -= frame_time;

		if (time < 0)
		{
			time = m_Sprite_time;
			cur_sprite = (cur_sprite + 1) % m_nSprites_cnt;

			if (cur_sprite == 0)
				cycles--;
		}

		return cur_sprite;
	}

	inline void add_cycle(int i)
	{
		cycles += i;
	}
};