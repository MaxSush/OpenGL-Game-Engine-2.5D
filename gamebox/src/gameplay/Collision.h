#pragma once

#include <glm/glm.hpp>

#include <cmath>
#include <vector>

struct Body
{
	glm::vec2 position{ 0.0f };
	glm::vec2 half_size{ 0.0f };
};

struct TransformComponent
{
	glm::vec2 position{ 0.0f };
	glm::vec2 half_size{ 0.5f };
	glm::vec2 velocity{ 0.0f };
	bool grounded = false;
};

struct Solid
{
	Body b{};
};

struct MoveResult
{
	bool hit_x = false;
	bool hit_y = false;
};

inline bool Overlaps(const Body& a, const Body& b)
{
	constexpr float skin = 0.0001f;
	return (a.half_size.x + b.half_size.x - std::abs(a.position.x - b.position.x)) > skin &&
		(a.half_size.y + b.half_size.y - std::abs(a.position.y - b.position.y)) > skin;
}

inline Body BoundsOf(const TransformComponent& b)
{
	return { b.position, b.half_size };
}

MoveResult MoveAndCollide(TransformComponent& b, const std::vector<Solid>& solids, float dt);