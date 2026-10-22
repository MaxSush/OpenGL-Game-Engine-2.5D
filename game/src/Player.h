#pragma once

#include <glm/glm.hpp>

#include <vector>

#include "Collision.h"
#include "Tuning.h"

struct PlayerIntent
{
	float move_x = 0.0f;
	bool jump_held = false;
	bool jump_pressed = false;
};

struct Player
{
	glm::vec2 position{ 0.0f };
	glm::vec2 velocity{ 0.0f };
	glm::vec2 half_size{ 0.4f, 0.5f };
	bool grounded = false;
	float coyote_timer = 0.0f;
	float jump_buffer_timer = 0.0f;
};

void StepPlayer(Player& p, PlayerIntent& in, const Tuning& t, const std::vector<AABB>& solids, float dt);