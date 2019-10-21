#pragma once
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "render_stuff.h"

inline bool load_texture(const std::string filename, uint32_t*& texture, size_t& text_size, size_t& text_cnt) {
	int nchannels = -1, w, h;
	unsigned char* pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0);
	if (!pixmap) {
		//std::cerr << "Error: can not load the textures" << std::endl;
		return false;
	}

	if (nchannels != 4 && nchannels != 3) {
		//std::cerr << "Error: the texture must be a 32 bit image" << std::endl;
		stbi_image_free(pixmap);
		return false;
	}

	text_cnt = w / h;
	text_size = w / text_cnt;
	if (w != h * int(text_cnt)) {
		//std::cerr << "Error: the texture file must contain N square textures packed horizontally" << std::endl;
		stbi_image_free(pixmap);
		return false;
	}

	texture = new uint32_t[w * h];

	if (nchannels == 4)
	{
		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				uint8_t r = pixmap[(i + j * w) * 4 + 0];
				uint8_t g = pixmap[(i + j * w) * 4 + 1];
				uint8_t b = pixmap[(i + j * w) * 4 + 2];
				uint8_t a = pixmap[(i + j * w) * 4 + 3];
				texture[i + j * w] = pack_color(r, g, b, a);
			}
		}
	}
	else if (nchannels == 3)
	{
		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				uint8_t r = pixmap[(i + j * w) * 3 + 0];
				uint8_t g = pixmap[(i + j * w) * 3 + 1];
				uint8_t b = pixmap[(i + j * w) * 3 + 2];
				//uint8_t a = pixmap[(i + j * w) * 4 + 3];
				texture[i + j * w] = pack_color(r, b, g, 255);
			}
		}
	}
	stbi_image_free(pixmap);
	return true;
}


inline void texture_column(uint32_t* out_colum, const uint32_t* img, const size_t texsize, const size_t ntextures, const size_t texid, const size_t texcoord, const size_t column_height) {
	const size_t img_w = texsize * ntextures;
	const size_t img_h = texsize;
	//uint32_t* column = (uint32_t*)_alloca(sizeof(uint32_t) * column_height);
	//assert(column);

	for (size_t y = 0; y < column_height; y++) {
		size_t pix_x = texid * texsize + texcoord;
		size_t pix_y = (y * texsize) / column_height;
		out_colum[y] = img[pix_x + pix_y * img_w];
	}
}

