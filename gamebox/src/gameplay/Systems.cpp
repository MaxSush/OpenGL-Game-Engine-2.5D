#include "Systems.h"

#include <engine/gameplay/scene.h>

#include <algorithm>
#include <cmath>

#include <engine/physics/collision.h>
#include "Components.h"

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

	void StepPlayer(Player& p, Engine::Transform& tr, Engine::Collider& col, Engine::Motion& m, 
		PlayerIntent& in, const Tuning& t, const std::vector<Engine::StaticBox>& solids, float dt)
	{
		// Timers (grounded comes from last step's collision)
		p.coyote_timer = (m.grounded) ? t.coyote_time : std::max(0.0f, p.coyote_timer - dt);
		p.jump_buffer_timer = (in.jump_pressed) ? t.jump_buffer_time : std::max(0.0f, p.jump_buffer_timer - dt);
		in.jump_pressed = false;

		// Horizontal: accelerate toward the target speed, or apply friction when there is no input.
		const float target = in.move_x * t.max_speed;
		const float rate = (in.move_x != 0.0f) ? t.accel : t.friction;
		m.velocity.x = MoveToward(m.velocity.x, target, rate * dt);

		// Jump: buffered press + coyote window.
		if (p.jump_buffer_timer > 0.0f && p.coyote_timer > 0.0f)
		{
			m.velocity.y = t.jump_velocity;
			p.jump_buffer_timer = 0.0f;
			p.coyote_timer = 0.0f;
		}

		// Gravity (heavier when rising and jump released: variable jump height)
		float g = t.gravity;
		if (m.velocity.y > 0.0f && !in.jump_held)
		{
			g *= t.release_gravity_mult;
		}
		m.velocity.y = std::max(m.velocity.y + g * dt, t.max_fall_speed);

		Engine::MoveAndCollide(tr, col, m, solids, dt);
	}
}

namespace
{
	bool HasGroundAhead(const Engine::Transform& tr, const Engine::Collider& col, float direction, 
		std::span<const Engine::StaticBox> solids)
	{
		constexpr float reach = 0.15f;
		const Engine::StaticBox probe{
			{ tr.position.x + direction * (col.half_size.x + reach), tr.position.y - col.half_size.y - 0.05f },
			{0.05f, 0.10f}
		};

		for (const Engine::StaticBox& s : solids)
		{
			if (Engine::Overlaps(probe, s))
			{
				return true;
			}
		}
		return false;
	}

	void StepEnemy(Enemy& e, Engine::Transform& tr, Engine::Collider& col, Engine::Motion& m,
		const Tuning& t, const std::vector<Engine::StaticBox>& solids, float dt)
	{
		m.velocity.y = std::max(m.velocity.y + t.gravity * dt, t.max_fall_speed);

		if (m.grounded && !HasGroundAhead(tr, col, e.direction, solids))
		{
			e.direction = -e.direction;
		}

		m.velocity.x = e.direction * t.enemy_speed;
		const Engine::MoveResult hit = Engine::MoveAndCollide(tr, col, m, solids, dt);

		if (hit.hit_x)
			e.direction = -e.direction;
	}
}

void StepPlayers(Engine::Scene& scene, PlayerIntent& in, const Tuning& t, const std::vector<Engine::StaticBox>& solids, float dt)
{
	scene.Each<Player, Engine::Transform, Engine::Collider, Engine::Motion>(
		[&](Engine::Entity, Player& p, Engine::Transform& tr, Engine::Collider& col, Engine::Motion& m)
		{
			StepPlayer(p, tr, col, m, in, t, solids, dt);
		});

}

void StepEnemies(Engine::Scene& scene, const Tuning& t, const std::vector<Engine::StaticBox>& solids, float dt)
{
	scene.Each<Enemy, Engine::Transform, Engine::Collider, Engine::Motion>
		([&](Engine::Entity, Enemy& e, Engine::Transform& tr, Engine::Collider& col, Engine::Motion& m)
		{
			StepEnemy(e, tr, col, m, t, solids, dt);
		});
}
