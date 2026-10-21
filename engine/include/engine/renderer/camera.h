#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
	struct Camera
	{
		glm::vec2 position{ 0.0f,0.0f };
		float vertical_size = 9.0f;
		float aspect = 16.0f / 9.0f;

		void SetViewportSize(int width, int height)
		{
			if (width > 0 && height > 0)
			{
				aspect = static_cast<float>(width) / static_cast<float>(height);
			}
		}

		glm::mat4 ViewProjection() const
		{
			const float hh = vertical_size * 0.5f;
			const float hw = hh * aspect;
			return glm::ortho(position.x - hw, position.x + hw, position.y - hh, position.y + hh, -1.0f, 1.0f);
		}
	};
}