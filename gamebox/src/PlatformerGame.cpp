#include "PlatformerGame.h"

#include <engine/renderer/renderer.h>
#include "gameplay/Systems.h"
#include "gameplay/Components.h"

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

	Engine::World& world = App().GetWorld();
	world.CreateScene("MainLevel");
	world.SetActiveScene("MainLevel");

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

	StepCameraFollow(ActiveScene(), m_level.bounds_min, m_level.bounds_max, dt);
}

void PlatformerGame::OnFixedUpdate(float step)
{
	if (m_progress.state != GameState::PLAYING)
	{
		return;
	}

	Engine::Scene& scene = ActiveScene();
	StepPlayers(scene, m_intent, m_tuning, m_level.solids, step);
	StepEnemies(scene, m_tuning, m_level.solids, step);

	ResolveInteractions(ActiveScene(), m_level, m_progress);
}

void PlatformerGame::OnRender()
{
	auto& r = App().GetRenderer();
	r.BeginScene(m_camera);

	Engine::Scene& scene = ActiveScene();

	for (const Solid& s : m_level.solids)
	{
		r.DrawQuad(s.b.position, s.b.half_size * 2.0f, { 0.35f, 0.35f, 0.40f, 1.0f });
	}
	
	scene.Each<Hazard>([&](Engine::Entity, Hazard& h)
		{
			r.DrawQuad(h.b.position, h.b.half_size * 2.0f, { 1.0f, 0.42f, 0.05f, 1.0f });
		});

	scene.Each<Goal>([&](Engine::Entity, Goal& g)
		{
			r.DrawQuad(g.b.position, g.b.half_size * 2.0f, { 0.16f, 0.75f, 0.44f, 1.0f });
		});

	scene.Each<Enemy>([&](Engine::Entity, Enemy& e)
		{
			r.DrawQuad(e.transform.position, e.transform.half_size * 2.0f, { 0.44f, 0.08f, 0.69f, 1.0f });
		});

	scene.Each<Pickup>([&](Engine::Entity, Pickup& p)
		{
			r.DrawQuad(p.position, glm::vec2{ kPickupHalf * 2.0f }, { 0.85f, 0.65f, 0.13f, 1.0f });
		});

	r.DrawQuad(m_player.GetComponent<Player>().transform.position, m_player.GetComponent<Player>().transform.half_size * 2.0f, {0.0f, 0.81f, 0.82f,1.0f});
	
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
	TransformComponent t = m_player.GetComponent<Player>().transform;
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
		ui.Text("Player Pos: [%f, %f]", t.position.x, t.position.y);
		ui.Text("Player vel: [%f, %f]", t.velocity.x, t.velocity.y);
		ui.Text("Camera: [%.2f, %.2f]", m_camera.position.x, m_camera.position.y);


		ui.Separator();
		if (ui.Button("Restart Level"))
		{
			LoadLevel();
		}
	}
	ui.End();

	if (ui.Begin("Stats"))
	{
		ui.Text("State: %s (%s)", ToString(m_progress.state), ToString(m_progress.cause));
		ui.Text("Coins: %d / %d", m_progress.coins, static_cast<int>(m_coins_total));
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

	Engine::Scene& scene = ActiveScene();
	scene.Clear();

	m_player = scene.CreateEntity("Player");
	Player p;
	m_intent = PlayerIntent{};
	p.transform.position = { fresh.spawn_feet.x, fresh.spawn_feet.y + p.transform.half_size.y };
	m_player.AddComponent<Player>(p);
	m_player.AddComponent<CameraFollowComponent>(m_camera);

	m_progress = Progress{};

	m_camera.vertical_size = m_level.view_height;
	
	SnapCamera(m_camera, p.transform.position, m_level.bounds_min, m_level.bounds_max);

	for (const Pickup& c : fresh.pickups)
	{
		scene.CreateEntity("Coin").AddComponent<Pickup>(c);
	}
	for (const Hazard& h : fresh.hazards)
	{
		scene.CreateEntity("Hazard").AddComponent<Hazard>(h);
	}
	for (const Goal& g : fresh.goals)
	{
		scene.CreateEntity("Goals").AddComponent<Goal>(g);
	}
	for (const Enemy& e : fresh.enemies)
	{
		scene.CreateEntity("Enemy").AddComponent<Enemy>(e);
	}

	m_coins_total = fresh.pickups.size();

	fresh.pickups.clear();
	fresh.hazards.clear();
	fresh.goals.clear();
	fresh.enemies.clear();

	m_level = std::move(fresh);
	return true;
}

Engine::Scene& PlatformerGame::ActiveScene()
{
	return App().GetWorld().GetActiveScene();
}
