#include "Collision.h"

// Moves the player by velocity * dt, one axis at a time, resolving against solids.
// Sets p.grounded if the player landed on something this step.
MoveResult MoveAndCollide(TransformComponent& b, const std::vector<Solid>& solids, float dt)
{
	MoveResult result;
	b.grounded = false;

	// X axis
	const float vx = b.velocity.x;
	b.position.x += vx * dt;
	for (const Solid& s : solids)
	{
		if (!Overlaps(BoundsOf(b), s.b))
		{
			continue;
		}

		if (vx > 0.0f)
		{
			b.position.x = s.b.position.x - s.b.half_size.x - b.half_size.x;   // hit the left side of the solid
			result.hit_x = true;
		}
		else if (vx < 0.0f)
		{
			b.position.x = s.b.position.x + s.b.half_size.x + b.half_size.x;   // hit the right side of the solid
			result.hit_x = true;
		}
		if (result.hit_x)
		{
			b.velocity.x = 0.0f;
		}
	}

	// Y axis
	const float vy = b.velocity.y;
	b.position.y += vy * dt;
	for (const Solid& s : solids)
	{
		if (!Overlaps(BoundsOf(b), s.b))
		{
			continue;
		}

		if (vy < 0.0f)
		{
			b.position.y = s.b.position.y + s.b.half_size.y + b.half_size.y;  // landed on top
			b.grounded = true;
			result.hit_y = true;
		}
		else if (vy > 0.0f)
		{
			b.position.y = s.b.position.y - s.b.half_size.y - b.half_size.y;  // bumped head
			result.hit_y = true;
		}
		if (result.hit_y)
		{
			b.velocity.y = 0.0f;
		}
	}
	return result;
}
