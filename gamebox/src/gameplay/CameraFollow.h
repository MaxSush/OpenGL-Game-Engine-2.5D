#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	struct Camera;
}

struct CameraFollowParams
{
	glm::vec2 dead_zone_half{ 1.5f,1.0f };
	float smoothing = 8.0f;
};

// Puts the camera on the target immediately (level load, restart), clamped to the level.
void SnapCamera(Engine::Camera& cam, glm::vec2 target, glm::vec2 level_min, glm::vec2 level_max);

// Dead zone, then exponential smoothing, then clamp. Call once per rendered frame.
void UpdateCameraFollow(Engine::Camera& cam, glm::vec2 target, glm::vec2 level_min, glm::vec2 level_max,
	const CameraFollowParams& params, float dt);