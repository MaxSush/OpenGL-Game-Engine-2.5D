#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	struct Camera;
}

struct Tuning
{
	float gravity = -30.f;
	float release_gravity_mult = 2.5f;
	float jump_velocity = 12.0f;
	float max_speed = 7.0f;
	float accel = 60.0f;
	float friction = 50.0f;
	float max_fall_speed = -25.0f;
	float enemy_speed = 2.0f;
	float coyote_time = 0.10f;
	float jump_buffer_time = 0.10f;
};

struct PlayerIntent
{
	float move_x = 0.0f;
	bool jump_held = false;
	bool jump_pressed = false;
};

struct TransformComponent
{
	glm::vec2 position{ 0.0f };
	glm::vec2 half_size{ 0.5f };
	glm::vec2 velocity{ 0.0f };
	bool grounded = false;
};

struct CameraFollowComponent
{
	Engine::Camera& camera;
	glm::vec2 dead_zone_half{ 1.5f,1.0f };
	float smoothing = 8.0f;
};

struct Body
{
	glm::vec2 position{ 0.0f };
	glm::vec2 half_size{ 0.0f };
};