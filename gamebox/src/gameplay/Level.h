#pragma once

#include <glm/glm.hpp>

#include "Collision.h"

#include <vector>

struct Level
{
	std::vector<AABB> solids;
	glm::vec2 spawn{ 0.0f, 0.0f };
	float kill_y = -9.0f;
	float world_bound = 12.0f;
	float view_height = 12.0f;
};

Level BuildLevel();