#pragma once

#include <cstdint>

namespace Engine
{
	struct FrameTime
	{
		float delta = 0.0f;
		double total = 0.0;
		float fixed_step = 1.0f / 60.0f;
		float fps = 0.0f;
		uint64_t frame = 0;
	};
}