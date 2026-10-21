#pragma once
#include <engine/application.h>
#include <engine/game.h>
#include <engine/core/logger/log.h>
#include <engine/input/input.h>
#include <engine/renderer/camera.h>

class PlatformerGame final : public Engine::Game
{
public:
	bool OnStart() override;
	void OnUpdate(float dt) override;
	void OnShutdown() override;
	void OnResize(int width, int height) override;
	void OnRender() override;

private:
	Engine::InputActionId m_move_left{};
	Engine::InputActionId m_move_right{};
	Engine::InputActionId m_jump{};

	Engine::Camera m_camera;
};