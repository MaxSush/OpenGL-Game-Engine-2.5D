#pragma once

#include <vector>

#include "Tuning.h"

namespace Engine
{
	class Scene;
	struct StaticBox;
}

void StepPlayers(Engine::Scene& scene, PlayerIntent& in, const Tuning& t, const std::vector<Engine::StaticBox>& solids, float dt);

void StepEnemies(Engine::Scene& scene, const Tuning& t, const std::vector<Engine::StaticBox>& solids, float dt);

