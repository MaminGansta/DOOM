#pragma once
#include "bullet.h"
#include "../animation/animation.h" 

struct Imp_bullet : public Bullet
{
	inline Imp_bullet(int pos_x, int pos_y, int angle, int speed): Bullet(pos_x, pos_y, angle, speed) {};
};