#pragma once
#include <engine/application.h>
#include <engine/game.h>
#include <engine/core/logger/log.h>
#include <engine/input/input.h>
#include <engine/renderer/camera.h>
#include <engine/gameplay/entity.h>

#include "gameplay/Level.h"
#include "gameplay/Rules.h"

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

	size_t m_coins_total = 0;

private:
	bool LoadLevel();
	Engine::Scene& ActiveScene();
};