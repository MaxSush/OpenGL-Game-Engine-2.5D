#pragma once

#include <vector>

#include "Collision.h"
#include "Tuning.h"

namespace Engine
{
	class Scene;
}

void StepPlayers(Engine::Scene& scene, PlayerIntent& in, const Tuning& t, const std::vector<Solid>& solids, float dt);

void StepEnemies(Engine::Scene& scene, const Tuning& t, const std::vector<Solid>& solids, float dt);

