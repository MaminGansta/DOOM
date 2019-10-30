#pragma once

#include <cstdint>

struct Animation
{
	int m_nSprite_time;
	int m_nSprites_cnt;


	inline Animation(int cnt, int time): m_nSprites_cnt(cnt), m_nSprite_time(time){}

};