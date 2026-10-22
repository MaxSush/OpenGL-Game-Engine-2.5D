#include "PlatformerGame.h"

#include <engine/renderer/renderer.h>

void PlatformerGame::OnResize(int width, int height)
{
	m_camera.SetViewportSize(width, height);
}

bool PlatformerGame::OnStart()
{
	auto& actions = App().GetInput().GetActionMap();
	m_move_x = actions.RegisterAction("MoveX", Engine::InputActionType::Scalar);
	m_jump = actions.RegisterAction("Jump", Engine::InputActionType::Digital);

	Engine::InputBinding b;
	b.source = Engine::InputBindingSource::Key;

	b.action = m_move_x;	b.key = Engine::Key::A;     b.scale = -1.0f;	actions.AddBinding(b);
	b.action = m_move_x;	b.key = Engine::Key::D;     b.scale = 1.0f;		actions.AddBinding(b);
	b.action = m_jump;      b.key = Engine::Key::Space;  b.scale = 1.0f;	actions.AddBinding(b);

	auto& contexts = App().GetInput().GetContexts();
	auto ctx = contexts.RegisterContext("Game", 0, Engine::InputConsumptionPolicy::Consume);
	contexts.PushContext(ctx);

	m_level = BuildLevel();
	m_camera.position = { 0.0f, 0.0f };
	m_camera.vertical_size = m_level.view_height;

	ResetPlayer();

	LOG_INFO("Game started");
	return true;
}

void PlatformerGame::OnUpdate(float dt)
{
	auto& input = App().GetInput();

	if (input.WasKeyPressed(Engine::Key::Escape))
	{
		App().RequestQuit();
	}

	if (input.WasKeyPressed(Engine::Key::R))
	{
		ResetPlayer();
	}

	m_intent.move_x = input.GetAxis(m_move_x);
	m_intent.jump_held = input.IsActionDown(m_jump);

	if (input.WasActionPressed(m_jump))
	{
		m_intent.jump_pressed = true;
	}
}

void PlatformerGame::OnFixedUpdate(float step)
{
	StepPlayer(m_player, m_intent, m_tuning, m_level.solids, step);

	if (m_player.position.y < m_level.kill_y || std::abs(m_player.position.x) > 12.0f)
	{
		ResetPlayer();
	}
}

void PlatformerGame::OnRender()
{
	auto& r = App().GetRenderer();
	r.BeginScene(m_camera);

	const glm::vec4 solid_color{ 0.35f, 0.35f, 0.40f, 1.0f };
	for (const AABB& s : m_level.solids)
	{
		r.DrawQuad(s.center, s.half * 2.0f, solid_color);
	}
	r.DrawQuad(m_player.position, m_player.half_size * 2.0f, { 0.90f,0.40f,0.20f,1.0f });
	
	r.EndScene();
}

void PlatformerGame::OnShutdown()
{
	LOG_INFO("Game shutdown");
}

void PlatformerGame::ResetPlayer()
{
	m_player = Player{};
	m_player.position = m_level.spawn;
	m_intent = PlayerIntent{};
}
