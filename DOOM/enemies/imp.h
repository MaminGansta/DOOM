#pragma once
#include "enemy.h"

struct Imp: public Enemy
{
	Imp(int hp, float x, float y, float angle): Enemy(hp, x, y, angle) {};

};