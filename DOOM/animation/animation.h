#pragma once

#include <cstdint>

struct Animation
{
	int m_nSprite_time;
	int m_nSprites_cnt;
	int time;
	int cur_sprite{0};
	int cycles{ 0 };


	inline Animation(int cnt, int time): m_nSprites_cnt(cnt), m_nSprite_time(time), time(time){}

	inline int sprite(int frame_time)
	{
		if (cycles == 0)
			return cur_sprite;

		time -= frame_time;

		if (time < 0)
		{
			time = m_nSprite_time;
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