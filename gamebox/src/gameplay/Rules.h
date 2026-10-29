#pragma once

#include "Progress.h"

struct Level;

namespace Engine
{
	class Scene;
}

// Once per fixed step, after everything has moved. Does nothing unless state == PLAYING.
// Order: pickups, then death (hazard, enemy, fall), then the goal. Death wins over the goal.
void ResolveInteractions(Engine::Scene& scene, const Level& level, Progress& progress);
