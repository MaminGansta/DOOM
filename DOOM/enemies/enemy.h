#pragma once
#include <cstdint>

struct Enemy
{
	int m_hp;
	float m_pos_x;
	float m_pos_y;
	float m_agle;
	float m_distance{1};

	static uint32_t* sprites;

	inline Enemy(int hp, float x, float y, float angle) : m_hp(hp), m_pos_x(x), m_pos_y(y), m_agle(angle){}

	inline ~Enemy()
	{
		delete[] sprites;
	}
};

uint32_t* Enemy::sprites = NULL;

