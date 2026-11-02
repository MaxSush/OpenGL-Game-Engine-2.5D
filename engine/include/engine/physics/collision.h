#pragma once

#include <engine/engine_api.h>
#include <engine/gameplay/transform.h>

#include <glm/glm.hpp>

#include <cmath>
#include <span>

namespace Engine
{
	struct StaticBox
	{
		glm::vec2 position{ 0.0f };
		glm::vec2 half_size{ 0.5f };
	};

	struct Collider
	{
		glm::vec2 half_size{ 0.5f };
	};

	struct Motion
	{
		glm::vec2 velocity{ 0.0f };
		bool grounded = false;
	};

	struct MoveResult
	{
		bool hit_x = false;
		bool hit_y = false;
	};

	inline bool Overlaps(const StaticBox& a, const StaticBox& b)
	{
		constexpr float skin = 0.0001f;
		return (a.half_size.x + b.half_size.x - std::abs(a.position.x - b.position.x)) > skin &&
			(a.half_size.y + b.half_size.y - std::abs(a.position.y - b.position.y)) > skin;
	}

	inline StaticBox BoundsOf(const Transform& t, const Collider& c)
	{ 
		return { t.position, c.half_size }; 
	}

	ENGINE_API MoveResult MoveAndCollide(Transform& transform, const Collider& collider, Motion& motion, std::span<const StaticBox> solids, float dt);

}