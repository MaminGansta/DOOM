
enum Byllet_type{ IMP_BULLET, BULLET_TYPES_COUNT };

Sprites bullet_spites[BULLET_TYPES_COUNT];


struct Bullet 
{
	float pos_x;
	float pos_y;
	float angle;
	float speed;
	float distance;

	uint8_t sprite_id;

	Bullet() : pos_x(0), pos_y(0), angle(0), speed(0), distance(10), sprite_id(0) {}
	Bullet(float pos_x, float pos_y, float angle, float speed): pos_x(pos_x), pos_y(pos_y), angle(angle), speed(speed), distance(10) {}
};


template <size_t max_size>
struct Bullet_buffer
{
	size_t actives;
	Bullet buffer[max_size];

	Bullet_buffer() : actives(0) {}

	void add(float pos_x, float pos_y, float angle, float speed)
	{
		if (actives == max_size) return;
		buffer[actives++] = Bullet(pos_x, pos_y, angle, speed);
	}

	void remove(size_t indx)
	{
		if (actives == 0 || indx > actives || indx < 0) return;
		using std::swap;
		swap(buffer[indx], buffer[--actives]);
	}

	Bullet& operator [] (size_t indx) { return buffer[indx]; }

	Bullet* begin() { return buffer; }
	Bullet* end() { return buffer + actives; }

};