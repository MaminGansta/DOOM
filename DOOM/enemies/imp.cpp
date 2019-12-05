struct Imp: public Enemy
{
	Imp(int hp, float x, float y, float angle): Enemy(hp, x, y, angle) {};
};

inline void imp_behavior(Enemy* imp, float pl_x, float pl_y, float frame_time, std::vector<Bullet*>& bullets)
{
	if (!imp->visible)
		return;

	imp->attack_deley_time -= frame_time;

	if (imp->attack_deley_time < 0)
		imp->attack_deley_time = imp->attack_deley;
	else
		return;

	float bullet_a = atan2(imp->m_pos_y - pl_y, imp->m_pos_x - pl_x);

	bullets.push_back(new Imp_bullet(imp->m_pos_x, imp->m_pos_y, bullet_a + PI, 8));
}