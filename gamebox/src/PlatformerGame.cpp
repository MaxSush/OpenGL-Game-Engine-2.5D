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

	b.action = m_move_x;	b.key = Engine::Key::Left;		b.scale = -1.0f;	actions.AddBinding(b);
	b.action = m_move_x;	b.key = Engine::Key::Right;     b.scale = 1.0f;		actions.AddBinding(b);
	b.action = m_jump;      b.key = Engine::Key::Space;		b.scale = 1.0f;		actions.AddBinding(b);

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

void PlatformerGame::OnUpdate(float /* dt */)
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

	if (m_player.position.y < m_level.kill_y || std::abs(m_player.position.x) > m_level.world_bound)
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

void PlatformerGame::OnDebugUI(Engine::DebugUI& ui)
{
	if (ui.Begin("Tuning"))
	{
		ui.SliderFloat("Gravity", m_tuning.gravity, -80.0f, -1.0f);
		ui.SliderFloat("Release gravity multiplier", m_tuning.release_gravity_mult, 1.0f, 4.0f);
		ui.SliderFloat("Jump velocity", m_tuning.jump_velocity, 1.0f, 25.0f);
		ui.SliderFloat("Max speed", m_tuning.max_speed, 1.0f, 20.0f);
		ui.SliderFloat("Acceleration", m_tuning.accel, 1.0f, 120.0f);
		ui.SliderFloat("Friction", m_tuning.friction, 1.0f, 120.0f);
		ui.SliderFloat("Max fall speed", m_tuning.max_fall_speed, -60.0f, -1.0f);
		ui.SliderFloat("Coyote time", m_tuning.coyote_time, 0.0f, 0.3f);
		ui.SliderFloat("Jump buffer time", m_tuning.jump_buffer_time, 0.0f, 0.3f);

		ui.Separator();
		ui.Text("FPS: %.1f", App().GetTime().fps);

		const auto& stats = App().GetRenderer().GetStats();
		ui.Text("Draw calls: %u", static_cast<unsigned>(stats.draw_calls));
		ui.Text("Quads: %u", static_cast<unsigned>(stats.quad_count));
		
		ui.Separator();
		ui.Text("Player Pos: [%f, %f]", m_player.position.x, m_player.position.y);
		ui.Text("Player vel: [%f, %f]", m_player.velocity.x, m_player.velocity.y);

		ui.Separator();
		if (ui.Button("Reset player"))
		{
			ResetPlayer();
		}
	}
	ui.End();
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
