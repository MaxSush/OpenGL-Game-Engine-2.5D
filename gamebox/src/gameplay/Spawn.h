#pragma once

#include <engine/gameplay/entity.h>
#include "Level.h"

namespace Engine
{
	class Scene;
}

struct SpawnResult
{
	Engine::Entity player;
	int coins = 0;
};

SpawnResult SpawnLevel(Engine::Scene& scene, const Level& level);