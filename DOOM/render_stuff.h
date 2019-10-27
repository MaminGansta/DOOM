#pragma once
#include <cstdint>
#include <cassert>

#define win_h surface.height
#define win_w surface.width

struct Render_State {
	int height, width;
	uint32_t* memory;

	BITMAPINFO bitmap_info;
};


inline uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255)
{
	return ((uint32_t)a << 24) + ((uint32_t)r << 16) + ((uint32_t)b << 8) + g;
}

void unpack_color(const uint32_t& color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
	g = (color >> 0) & 255;
	b = (color >> 8) & 255;
	r = (color >> 16) & 255;
	a = (color >> 24) & 255;
}

inline void draw_rectangle(Render_State* surface, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color)
{
	for (size_t i = 0; i < w; i++)
	{
		for (size_t j = 0; j < h; j++) {
			size_t cx = x + i;
			size_t cy = y + j;
			assert(cx < surface->width && cy < surface->height);
			surface->memory[cx + cy * surface->width] = color;
		}
	}
}
