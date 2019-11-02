#pragma once
#include <cstdint>
#include "../animation/animation.h"

struct Enemy
{
	int m_hp;
	float m_pos_x;
	float m_pos_y;
	float m_agle;
	float m_distance{1};
	bool visible;
	Animation death_animation{5, 100000, 1};
	
	bool hit{ false };
	Animation hit_animation{ 1, 100000, 0 };

	static uint32_t* death;
	static uint32_t* sprites;

	inline Enemy(int hp, float x, float y, float angle) : m_hp(hp), m_pos_x(x), m_pos_y(y), m_agle(angle){}

};

uint32_t* Enemy::sprites = NULL;
uint32_t* Enemy::death = NULL;

