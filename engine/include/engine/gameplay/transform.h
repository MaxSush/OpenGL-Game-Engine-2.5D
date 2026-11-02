#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	struct Transform
	{
		glm::vec2 position{ 0.0f };
		glm::vec2 prev_position{ 0.0f };

		Transform() = default;
		explicit Transform(glm::vec2 p) 
			: position(p),
			prev_position(p)
		{ }

		void Teleport(glm::vec2 p)
		{
			position  = p;
			prev_position = p;
		}

		glm::vec2 Interpolated(float alpha) const
		{
			return prev_position + (position - prev_position) * alpha;
		}
	};
}