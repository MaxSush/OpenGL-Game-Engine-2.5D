#pragma once
#include <engine/application.h>
#include <engine/game.h>
#include <engine/core/logger/log.h>
#include <engine/input/input.h>
#include <engine/renderer/camera.h>

#include "Player.h"
#include "Tuning.h"
#include "Level.h"

class PlatformerGame final : public Engine::Game
{
public:
	// Inherited from Game
	void OnResize(int width, int height) override;
	bool OnStart() override;
	void OnUpdate(float dt) override;
	void OnFixedUpdate(float step) override;
	void OnRender() override;
	void OnShutdown() override;

private:
	Engine::InputActionId m_move_x{};
	Engine::InputActionId m_jump{};

	PlayerIntent m_intent{};
	Player m_player{};
	Level m_level{};
	Tuning m_tuning{};

	Engine::Camera m_camera;

private:
	void ResetPlayer();
};