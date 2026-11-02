#include <engine/physics/collision.h>

namespace Engine
{
	MoveResult MoveAndCollide(Transform& t, const Collider& c, Motion& m,
		std::span<const StaticBox> solids, float dt)
	{
		MoveResult result;
		m.grounded = false;

		const float vx = m.velocity.x;
		t.position.x += vx * dt;
		for (const StaticBox& s : solids)
		{
			if (!Overlaps(BoundsOf(t, c), s)) continue;
			if (vx > 0.0f) { t.position.x = s.position.x - s.half_size.x - c.half_size.x; result.hit_x = true; }
			else if (vx < 0.0f) { t.position.x = s.position.x + s.half_size.x + c.half_size.x; result.hit_x = true; }
			if (result.hit_x) m.velocity.x = 0.0f;
		}

		const float vy = m.velocity.y;
		t.position.y += vy * dt;
		for (const StaticBox& s : solids)
		{
			if (!Overlaps(BoundsOf(t, c), s)) continue;
			if (vy < 0.0f) { t.position.y = s.position.y + s.half_size.y + c.half_size.y; m.grounded = true; result.hit_y = true; }
			else if (vy > 0.0f) { t.position.y = s.position.y - s.half_size.y - c.half_size.y; result.hit_y = true; }
			if (result.hit_y) m.velocity.y = 0.0f;
		}
		return result;
	}

}