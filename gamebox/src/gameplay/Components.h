#pragma once

#include <glm/glm.hpp>

#include <engine/physics/collision.h>

constexpr float kPickupHalf = 0.25f;

struct Pickup
{
	glm::vec2 position{ 0.0f };
};

struct Hazard
{
	Engine::StaticBox b{};
};

struct Goal
{
	Engine::StaticBox b{};
};

struct Player
{
	float coyote_timer = 0.0f;
	float jump_buffer_timer = 0.0f;
};

struct Enemy
{
	float direction = -1.0f;
};
