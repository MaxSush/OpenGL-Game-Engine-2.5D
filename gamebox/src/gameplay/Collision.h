#pragma once

#include <glm/glm.hpp>

#include <cmath>
#include <vector>

struct Body
{
	glm::vec2 position{ 0.0f };
	glm::vec2 velocity{ 0.0f };
	glm::vec2 half_size{ 0.5f };
	bool grounded = false;
};

struct MoveResult
{
	bool hit_x = false;
	bool hit_y = false;
};

struct AABB
{
	glm::vec2 center{ 0.0f };
	glm::vec2 half{ 0.5f };
};

inline bool Overlaps(const AABB& a, const AABB& b)
{
	constexpr float skin = 0.0001f;
	return (a.half.x + b.half.x - std::abs(a.center.x - b.center.x)) > skin &&
		(a.half.y + b.half.y - std::abs(a.center.y - b.center.y)) > skin;
}

inline AABB BoundsOf(const Body& b)
{
	return { b.position, b.half_size };
}

MoveResult MoveAndCollide(Body& b, const std::vector<AABB>& solids, float dt);