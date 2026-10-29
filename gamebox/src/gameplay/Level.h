#pragma once

#include <glm/glm.hpp>

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "GameObjects.h"


inline constexpr float kPickupHalf = 0.25f;

struct Level
{
	std::vector<Solid> solids;
	std::vector<Pickup> pickups;
	std::vector<Hazard> hazards;
	std::vector<Goal> goals;
	std::vector<Enemy> enemies;

	glm::vec2 spawn_feet{ 0.0f };   // bottom-centre of the 'P' cell; the game adds the player's half height
	glm::vec2 bounds_min{ 0.0f };
	glm::vec2 bounds_max{ 0.0f };

	float kill_y = -3.0f;
	float view_height = 12.0f;
};

std::span<const std::string_view> Level1Rows();
// Row 0 is the TOP row. # solid, . empty, P player, C coin, ^ spike, G goal, E enemy.
bool ParseLevel(std::span<const std::string_view> rows, Level& out, std::string& error);