#pragma once

#include <glm/glm.hpp>

#include <cmath>
#include <vector>

struct Player;

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

void MoveAndCollide(Player& p, const std::vector<AABB>& solids, float dt);