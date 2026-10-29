#pragma once

#include <vector>

#include "GameObjects.h"

namespace Engine
{
	class Scene;
	struct Camera;
}

void StepPlayers(Engine::Scene& scene, PlayerIntent& in, const Tuning& t, const std::vector<Solid>& solids, float dt);

void StepEnemies(Engine::Scene& scene, const Tuning& t, const std::vector<Solid>& solids, float dt);

// Puts the camera on the target immediately (level load, restart), clamped to the level.
void SnapCamera(Engine::Camera& cam, glm::vec2 target, glm::vec2 level_min, glm::vec2 level_max);

// Dead zone, then exponential smoothing, then clamp. Call once per rendered frame.
void StepCameraFollow(Engine::Scene& scene, glm::vec2 level_min, glm::vec2 level_max, float dt);
