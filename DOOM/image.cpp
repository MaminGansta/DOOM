

struct Sprites
{
	Color* data;
	int height, width, whole_width;
	uint8_t count;

	// flag of loading result
	bool invalid = false;

	Sprites() : invalid(true), data(nullptr), height(0), width(0), count(0), whole_width(0) {}
	Sprites(const char* filename) { load(filename); }

	void load(const char*filename)
	{
		int nchannels = -1;
		uint8_t* pixmap = stbi_load(filename, &whole_width, &height, &nchannels, 0);
		if (!pixmap) {
			invalid = true;
			return;
		}

		if (nchannels != 4 && nchannels != 3) {
			stbi_image_free(pixmap);
			invalid = true;
			return;
		}

		// input texture must be squred
		count = whole_width / height;
		width = whole_width / count;
		if (whole_width != height * count) {
			stbi_image_free(pixmap);
			invalid = true;
			return;
		}

		data = new Color[whole_width * height];

		if (nchannels == 4)
		{
			for (int j = 0; j < height; j++)
			{
				for (int i = 0; i < width; i++)
				{
					uint8_t r = pixmap[(i + j * whole_width) * 4 + 0];
					uint8_t g = pixmap[(i + j * whole_width) * 4 + 1];
					uint8_t b = pixmap[(i + j * whole_width) * 4 + 2];
					uint8_t a = pixmap[(i + j * whole_width) * 4 + 3];
					data[i + j * whole_width] = Color(r, b, g, a);
				}
			}
		}
		else if (nchannels == 3)
		{
			for (int j = 0; j < height; j++)
			{
				for (int i = 0; i < whole_width; i++)
				{
					uint8_t r = pixmap[(i + j * whole_width) * 3 + 0];
					uint8_t g = pixmap[(i + j * whole_width) * 3 + 1];
					uint8_t b = pixmap[(i + j * whole_width) * 3 + 2];
					data[i + j * whole_width] = Color(r, b, g);
				}
			}
		}

		stbi_image_free(pixmap);
	}

	~Sprites() { delete[] data; };

	Sprites& operator = (Sprites&& other)
	{
		data = other.data;
		other.data = nullptr;
		width = other.width;
		height = other.height;
		count = other.count;
		return *this;
	}
};


inline void texture_column(Color* out_colum, Sprites& wall_textures, const size_t& texid, const size_t& texcoord, const size_t& column_height)
{
	const size_t img_w = wall_textures.whole_width;

	for (size_t y = 0; y < column_height; y++)
	{
		size_t pix_x = texid * wall_textures.width + texcoord;
		size_t pix_y = wall_textures.height - (y * wall_textures.height) / column_height - 1;
		out_colum[y] = wall_textures.data[pix_x + pix_y * img_w];
	}
}


inline void draw_rectangle(const size_t x, const size_t y, const size_t w, const size_t h, const Color& color)
{
	for (size_t i = 0; i < w; i++)
	{
		for (size_t j = 0; j < h; j++) {
			size_t cx = x + i;
			size_t cy = y + j;
			assert(cx < surface.width && cy < surface.height);
			surface.memory[cx + cy * surface.width] = color;
		}
	}
}


