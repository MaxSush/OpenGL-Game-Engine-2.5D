#pragma once

#include <glm/glm.hpp>

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <engine/physics/collision.h>


enum class SpawnKind
{
	PLAYER,
	COIN,
	SPIKE,
	GOAL,
	ENEMY
};

struct SpawnPoint
{
	SpawnKind kind;
	glm::vec2 feet;		// bottom-centre of the grid cell
};


struct Level
{
	std::vector<Engine::StaticBox> solids;

	std::vector<SpawnPoint> spawns;

	glm::vec2 bounds_min{ 0.0f };
	glm::vec2 bounds_max{ 0.0f };

	float kill_y = -3.0f;
	float view_height = 12.0f;
};

std::span<const std::string_view> Level1Rows();
// Row 0 is the TOP row. # solid, . empty, P player, C coin, ^ spike, G goal, E enemy.
bool ParseLevel(std::span<const std::string_view> rows, Level& out, std::string& error);
