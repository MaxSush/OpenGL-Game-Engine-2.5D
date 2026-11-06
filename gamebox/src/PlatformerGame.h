#pragma once

#include <engine/game.h>
#include <engine/gameplay/entity.h>
#include <engine/input/input_types.h>
#include <engine/renderer/camera.h>

#include "gameplay/CameraFollow.h"
#include "gameplay/Level.h"
#include "gameplay/Progress.h"
#include "gameplay/Tuning.h"

#include <array>

namespace Engine
{
	class DebugUI;
	class Scene;
}

class PlatformerGame final : public Engine::Game
{
public:
	// Inherited from Game
	void OnResize(int width, int height) override;
	bool OnStart() override;
	void OnUpdate(float dt) override;
	void OnFixedUpdate(float step) override;
	void OnRender() override;
	void OnDebugUI(Engine::DebugUI& ui) override;
	void OnShutdown() override;

private:
	Engine::InputActionId m_move_x{};
	Engine::InputActionId m_jump{};

	Engine::Entity m_player;

	Level m_level{};
	Tuning m_tuning{};
	Progress m_progress{};
	PlayerIntent m_intent{};

	Engine::Camera m_camera;
	CameraFollowParams m_follow{};

	size_t m_coins_total = 0;

private:
	void StartLevel();
	Engine::Scene& ActiveScene();

	// Debug
private:
	std::array<float, 120> m_dt_history{};
	std::size_t m_dt_index = 0;
	bool m_interpolate = true;
	bool m_camera_frozen = false;
	bool m_camera_interpolated = true;
};