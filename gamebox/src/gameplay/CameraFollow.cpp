#include "CameraFollow.h"

#include <engine/renderer/camera.h>

#include <algorithm>
#include <cmath>

namespace
{
	float ClampAxis(float v, float lo, float hi, float half_view)
	{
		if (hi - lo <= 2.0f * half_view)
		{
			return 0.5f * (lo + hi);
		}
		return std::clamp(v, lo + half_view, hi - half_view);
	}

	glm::vec2 ClampToLevel(const Engine::Camera& cam, glm::vec2 p, glm::vec2 lo, glm::vec2 hi)
	{
		const float half_h = cam.vertical_size * 0.5f;
		const float half_w = half_h * cam.aspect;      // depends on the window, so clamp every frame
		return { ClampAxis(p.x, lo.x, hi.x, half_w), ClampAxis(p.y, lo.y, hi.y, half_h) };
	}

	float DeadZoneGoal(float cam, float target, float half_zone)
	{
		const float d = target - cam;
		if (d > half_zone)  return target - half_zone;
		if (d < -half_zone) return target + half_zone;
		return cam;
	}
}	

void SnapCamera(Engine::Camera& cam, glm::vec2 target, glm::vec2 lo, glm::vec2 hi)
{
	cam.position = ClampToLevel(cam, target, lo, hi);
}

void UpdateCameraFollow(Engine::Camera& cam, glm::vec2 target, glm::vec2 lo, glm::vec2 hi,
	const CameraFollowParams& params, float dt)
{
	const glm::vec2 goal{
		DeadZoneGoal(cam.position.x, target.x, params.dead_zone_half.x),
		DeadZoneGoal(cam.position.y, target.y, params.dead_zone_half.y) 
	};

	const float k = 1.0f - std::exp(-params.smoothing * dt);   // frame-rate independent
	cam.position += (goal - cam.position) * k;

	cam.position = ClampToLevel(cam, cam.position, lo, hi);
}
