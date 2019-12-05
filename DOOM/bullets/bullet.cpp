struct Bullet 
{
	float pos_x;
	float pos_y;
	float angle;
	float speed;
	float distance;

	static uint32_t* sprite;

	inline Bullet(float pos_x, float pos_y, float angle, float speed): pos_x(pos_x), pos_y(pos_y), angle(angle), speed(speed) {}

};

uint32_t* Bullet::sprite = NULL;