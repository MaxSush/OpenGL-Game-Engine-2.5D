#pragma once

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
