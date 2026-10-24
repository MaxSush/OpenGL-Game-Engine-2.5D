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

	m_camera.position = { 0.0f, 0.0f };
	m_camera.vertical_size = m_level.view_height;

	const Engine::Dimension fb = App().GetWindow().GetFramebufferSize();
	m_camera.SetViewportSize(fb.width, fb.height);

	SnapCamera(m_camera, m_player.body.position, m_level.bounds_min, m_level.bounds_max);

	LOG_INFO("Game started");
	return LoadLevel();
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
		LoadLevel();
	}

	m_intent.move_x = input.GetAxis(m_move_x);
	m_intent.jump_held = input.IsActionDown(m_jump);

	if (input.WasActionPressed(m_jump))
	{
		if (m_progress.state == GameState::PLAYING)
		{
			m_intent.jump_pressed = true;
		}
	}

	UpdateCameraFollow(m_camera, m_player.body.position, m_level.bounds_min, m_level.bounds_max, m_follow, dt);
}

void PlatformerGame::OnFixedUpdate(float step)
{
	if (m_progress.state != GameState::PLAYING)
	{
		return;
	}

	StepPlayer(m_player, m_intent, m_tuning, m_level.solids, step);
	for (Enemy& e : m_level.enemies)
	{
		StepEnemy(e, m_tuning, m_level.solids, step);
	}

	ResolveInteractions(m_player, m_level, m_progress);
}

void PlatformerGame::OnRender()
{
	auto& r = App().GetRenderer();
	r.BeginScene(m_camera);

	for (const AABB& s : m_level.solids)
	{
		r.DrawQuad(s.center, s.half * 2.0f, { 0.35f, 0.35f, 0.40f, 1.0f });
	}
	
	for (const Hazard& h : m_level.hazards)
	{
		r.DrawQuad(h.box.center, h.box.half * 2.0f, { 1.0f, 0.42f, 0.05f, 1.0f });
	}

	for (const Goal& g : m_level.goals)
	{
		r.DrawQuad(g.box.center, g.box.half * 2.0f, { 0.16f, 0.75f, 0.44f, 1.0f });
	}

	for (const Enemy& e : m_level.enemies)
	{
		r.DrawQuad(e.body.position, e.body.half_size * 2.0f, { 0.44f, 0.08f, 0.69f, 1.0f });
	}

	for (const Pickup& p : m_level.pickups)
	{
		if (!p.collected)
		{
			r.DrawQuad(p.position, glm::vec2{ kPickupHalf * 2.0f }, { 0.85f, 0.65f, 0.13f, 1.0f });
		}
	}

	r.DrawQuad(m_player.body.position, m_player.body.half_size * 2.0f, { 0.0f, 0.81f, 0.82f,1.0f });
	
	if (m_progress.state != GameState::PLAYING)
	{
		const glm::vec2 view{ m_camera.aspect * m_camera.vertical_size, m_camera.vertical_size };
		const glm::vec4 tint = 
			(m_progress.state == GameState::WIN) ? glm::vec4{ 0.10f, 0.80f, 0.30f, 0.35f } : glm::vec4{ 0.80f, 0.10f, 0.10f, 0.35f };
		r.DrawQuad(m_camera.position, view, tint); 
	}
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
		ui.SliderFloat("Enemy speed", m_tuning.enemy_speed, 0.5f, 6.0f);
		ui.SliderFloat("Coyote time", m_tuning.coyote_time, 0.0f, 0.3f);
		ui.SliderFloat("Jump buffer time", m_tuning.jump_buffer_time, 0.0f, 0.3f);

		ui.Separator();
		ui.Text("FPS: %.1f", App().GetTime().fps);

		const auto& stats = App().GetRenderer().GetStats();
		ui.Text("Draw calls: %u", static_cast<unsigned>(stats.draw_calls));
		ui.Text("Quads: %u", static_cast<unsigned>(stats.quad_count));
		
		ui.Separator();
		ui.Text("Player Pos: [%f, %f]", m_player.body.position.x, m_player.body.position.y);
		ui.Text("Player vel: [%f, %f]", m_player.body.velocity.x, m_player.body.velocity.y);
		ui.Text("Camera: [%.2f, %.2f]", m_camera.position.x, m_camera.position.y);


		ui.Separator();
		if (ui.Button("Reset player"))
		{
			LoadLevel();
		}
	}
	ui.End();

	if (ui.Begin("Stats"))
	{
		ui.Text("State: %s (%s)", ToString(m_progress.state), ToString(m_progress.cause));
		ui.Text("Coins: %d / %d", m_progress.coins, static_cast<int>(m_level.pickups.size()));
	}
	ui.End();
}

void PlatformerGame::OnShutdown()
{
	LOG_INFO("Game shutdown");
}

bool PlatformerGame::LoadLevel()
{
	Level fresh;
	std::string error;
	if (!ParseLevel(Level1Rows(), fresh, error))
	{
		LOG_ERROR("PlatformerGame::LoadLevel() => LoadLevel: {}", error);
		return false;
	}

	m_level = std::move(fresh);
	m_intent = PlayerIntent{};
	m_player = Player{};
	m_player.body.position = { m_level.spawn_feet.x, m_level.spawn_feet.y + m_player.body.half_size.y };
	m_progress = Progress{};

	m_camera.vertical_size = m_level.view_height;
	SnapCamera(m_camera, m_player.body.position, m_level.bounds_min, m_level.bounds_max);
	return true;
}
