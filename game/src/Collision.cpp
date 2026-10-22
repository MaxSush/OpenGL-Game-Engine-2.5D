#include "Collision.h"

#include "Player.h"

// Moves the player by velocity * dt, one axis at a time, resolving against solids.
// Sets p.grounded if the player landed on something this step.
void MoveAndCollide(Player& p, const std::vector<AABB>& solids, float dt)
{
	p.grounded = false;

	// X axis
	p.position.x += p.velocity.x * dt;
	for (const AABB& s : solids)
	{
		if (!Overlaps({ p.position, p.half_size }, s))
		{
			continue;
		}

		if (p.velocity.x > 0.0f)
		{
			p.position.x = s.center.x - s.half.x - p.half_size.x;   // hit the left side of the solid
		}
		else if (p.velocity.x < 0.0f)
		{
			p.position.x = s.center.x + s.half.x + p.half_size.x;   // hit the right side of the solid
		}
		p.velocity.x = 0.0f;
	}

	// Y axis
	p.position.y += p.velocity.y * dt;
	for (const AABB& s : solids)
	{
		if (!Overlaps({ p.position, p.half_size }, s))
		{
			continue;
		}

		if (p.velocity.y < 0.0f)
		{
			p.position.y = s.center.y + s.half.y + p.half_size.y;  // landed on top
			p.grounded = true;
		}
		else if (p.velocity.y > 0.0f)
		{
			p.position.y = s.center.y - s.half.y - p.half_size.y;  // bumped head
		}
		p.velocity.y = 0.0f;
	}
}
