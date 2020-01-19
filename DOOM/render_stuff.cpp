
struct Color
{
	union
	{
		struct { uint8_t g, b, r, a; };
		uint8_t raw[4];
		uint32_t whole;
	};

	Color() = default;
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
	bool operator == (const Color& other) const { return (r == other.r && g == other.g && b == other.b && a == other.a); }
};


struct Render_State {
	int height, width;
	Color* memory;

	BITMAPINFO bitmap_info;
};