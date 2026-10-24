#include "Enemy.h"

namespace 
{
	bool HasGroundAhead(const Body& b, float direction, const std::vector<AABB>& solids)
	{
		constexpr float reach = 0.15f;
		const AABB probe{
			{ b.position.x + direction * (b.half_size.x + reach), b.position.y - b.half_size.y - 0.05f },
			{0.05f, 0.10f}
		};

		for (const AABB& s : solids)
		{
			if (Overlaps(probe, s))
			{
				return true;
			}
		}
		return false;
	}
}


void StepEnemy(Enemy& e, const Tuning& t, const std::vector<AABB>& solids, float dt)
{
	Body& b = e.body;

	b.velocity.y = std::max(b.velocity.y + t.gravity * dt, t.max_fall_speed);

	if (b.grounded && !HasGroundAhead(b, e.direction, solids))
	{
		e.direction = -e.direction;
	}

	b.velocity.x = e.direction * t.enemy_speed;
	const MoveResult hit = MoveAndCollide(b, solids, dt);

	if (hit.hit_x)
		e.direction = -e.direction;
}
