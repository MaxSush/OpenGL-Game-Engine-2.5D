#pragma once

#include <glm/glm.hpp>

#include "Collision.h"

inline constexpr float kPickupHalf = 0.25f;

struct Pickup
{
	glm::vec2 position{ 0.0f };
};

struct Hazard
{
	Body b{};
};

struct Goal
{
	Body b{};
};

struct Player
{
	TransformComponent transform{};
	float coyote_timer = 0.0f;
	float jump_buffer_timer = 0.0f;
};

struct Enemy
{
	TransformComponent transform;
	float direction = -1.0f;
};
