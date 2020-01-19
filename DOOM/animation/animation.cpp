struct Animation
{
	int sprites_count;
	float sprite_time;
	int cycles;

	float time;
	int cur_sprite;


	inline Animation(int cnt, float time, int cycles = 0): sprites_count(cnt), sprite_time(time), time(time), cycles(cycles), cur_sprite(0) {}

	inline int sprite(float frame_time)
	{
		if (cycles == 0)
			return cur_sprite;

		time -= frame_time;

		if (time < 0)
		{
			time = sprite_time;
			cur_sprite = (cur_sprite + 1) % sprites_count;

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