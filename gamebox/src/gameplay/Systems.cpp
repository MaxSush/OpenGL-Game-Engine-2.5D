#include "Systems.h"

#include <engine/gameplay/scene.h>
#include <engine/renderer/camera.h>

#include "Collision.h"

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

	void StepPlayer(Player& p, PlayerIntent& in, const Tuning& t, const std::vector<Solid>& solids, float dt)
	{
		TransformComponent& b = p.transform;
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

		MoveAndCollide(p.transform, solids, dt);
	}
}

namespace
{
	bool HasGroundAhead(const TransformComponent& b, float direction, const std::vector<Solid>& solids)
	{
		constexpr float reach = 0.15f;
		const TransformComponent probe{
			{ b.position.x + direction * (b.half_size.x + reach), b.position.y - b.half_size.y - 0.05f },
			{0.05f, 0.10f}
		};

		for (const Solid& s : solids)
		{
			if (Overlaps(BoundsOf(probe), s.b))
			{
				return true;
			}
		}
		return false;
	}

	void StepEnemy(Enemy& e, const Tuning& t, const std::vector<Solid>& solids, float dt)
	{
		TransformComponent& b = e.transform;

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
}

namespace
{
	float ClampAxis(float v, float lo, float hi, float half_view)
	{
		if (hi - lo <= 2.0f * half_view)
		{
			return 0.5f * (lo + hi);
		}
		return std::clamp(v, lo + half_view, hi - half_view);
	}

	glm::vec2 ClampToLevel(const Engine::Camera& cam, glm::vec2 p, glm::vec2 lo, glm::vec2 hi)
	{
		const float half_h = cam.vertical_size * 0.5f;
		const float half_w = half_h * cam.aspect;      // depends on the window, so clamp every frame
		return { ClampAxis(p.x, lo.x, hi.x, half_w), ClampAxis(p.y, lo.y, hi.y, half_h) };
	}

	float DeadZoneGoal(float cam, float target, float half_zone)
	{
		const float d = target - cam;
		if (d > half_zone)  return target - half_zone;
		if (d < -half_zone) return target + half_zone;
		return cam;
	}

	void UpdateCameraFollow(CameraFollowComponent& c, glm::vec2 target, glm::vec2 lo, glm::vec2 hi, float dt)
	{
		const glm::vec2 goal{
			DeadZoneGoal(c.camera.position.x, target.x, c.dead_zone_half.x),
			DeadZoneGoal(c.camera.position.y, target.y, c.dead_zone_half.y)
		};

		const float k = 1.0f - std::exp(-c.smoothing * dt);   // frame-rate independent
		c.camera.position += (goal - c.camera.position) * k;

		c.camera.position = ClampToLevel(c.camera, c.camera.position, lo, hi);
	}
}

void SnapCamera(Engine::Camera& cam, glm::vec2 target, glm::vec2 lo, glm::vec2 hi)
{
	cam.position = ClampToLevel(cam, target, lo, hi);
}

void StepCameraFollow(Engine::Scene& scene, glm::vec2 level_min, glm::vec2 level_max, float dt)
{
	scene.Each<Player, CameraFollowComponent>([&](Engine::Entity, Player& p, CameraFollowComponent& c)
		{
			UpdateCameraFollow(c, p.transform.position, level_min, level_max, dt);
		});
}

void StepPlayers(Engine::Scene& scene, PlayerIntent& in, const Tuning& t, const std::vector<Solid>& solids, float dt)
{
	scene.Each<Player>([&](Engine::Entity, Player& p)
		{
			StepPlayer(p, in, t, solids, dt);
		});
}

void StepEnemies(Engine::Scene& scene, const Tuning& t, const std::vector<Solid>& solids, float dt)
{
	scene.Each<Enemy>([&](Engine::Entity, Enemy& e)
		{
			StepEnemy(e, t, solids, dt);
		});
}
