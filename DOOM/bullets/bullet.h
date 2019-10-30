#pragma once
#include <cstdint>

struct Bullet 
{
	int pos_x;
	int pos_y;
	int angle;
	int speed;

	static uint32_t* sprite;

	inline Bullet(int pos_x, int pos_y, int angle, int speed): pos_x(pos_x), pos_y(pos_y), angle(angle), speed(speed) {}

};

uint32_t* Bullet::sprite = NULL;