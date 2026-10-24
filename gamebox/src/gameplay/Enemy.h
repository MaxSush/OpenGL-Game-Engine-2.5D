#pragma once

#include "Collision.h"
#include "Tuning.h"

#include <vector>

struct Enemy
{
	Body body{ {0.0f,0.0f}, {0.0f, 0.0f}, {0.4f, 0.4f}, false };
	float direction = -1.0f;
};

void StepEnemy(Enemy& e, const Tuning& t, const std::vector<AABB>& solids, float dt);