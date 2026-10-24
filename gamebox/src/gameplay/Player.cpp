#include "Player.h"

#include <algorithm>
#include <cmath>

namespace
{
	// Moves 'current' toward 'target' by at most 'max_delta', never overshooting.
	float MoveToward(float current, float target, float max_delta)
	{
		if (std::abs(target - current) <= max_delta)
		{
			return target;
		}
		return current + ((target > current) ? max_delta : -max_delta);
	}
}

void StepPlayer(Player& p, PlayerIntent& in, const Tuning& t, const std::vector<AABB>& solids, float dt)
{
	Body& b = p.body;
	// Timers (grounded comes from last step's collision)
	p.coyote_timer = (b.grounded) ? t.coyote_time : std::max(0.0f, p.coyote_timer - dt);
	p.jump_buffer_timer = (in.jump_pressed) ? t.jump_buffer_time : std::max(0.0f, p.jump_buffer_timer - dt);
	in.jump_pressed = false;

	// Horizontal: accelerate toward the target speed, or apply friction when there is no input.
	const float target = in.move_x * t.max_speed;
	const float rate = (in.move_x != 0.0f) ? t.accel : t.friction;
	b.velocity.x = MoveToward(b.velocity.x, target, rate * dt);

	// Jump: buffered press + coyote window.
	if (p.jump_buffer_timer > 0.0f && p.coyote_timer > 0.0f)
	{
		b.velocity.y = t.jump_velocity;
		p.jump_buffer_timer = 0.0f;
		p.coyote_timer = 0.0f;
	}

	// Gravity (heavier when rising and jump released: variable jump height)
	float g = t.gravity;
	if (b.velocity.y > 0.0f && !in.jump_held)
	{
		g *= t.release_gravity_mult;
	}
	b.velocity.y = std::max(b.velocity.y + g * dt, t.max_fall_speed);

	MoveAndCollide(p.body, solids, dt);
}