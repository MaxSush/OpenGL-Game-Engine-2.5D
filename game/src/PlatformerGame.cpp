#include "PlatformerGame.h"

bool PlatformerGame::OnStart()
{
	auto& actions = App().GetInput().GetActionMap();
	m_move_left = actions.RegisterAction("MoveLeft", Engine::InputActionType::Digital);
	m_move_right = actions.RegisterAction("MoveRight", Engine::InputActionType::Digital);
	m_jump = actions.RegisterAction("Jump", Engine::InputActionType::Digital);

	Engine::InputBinding b;
	b.source = Engine::InputBindingSource::Key;

	b.action = m_move_left;  b.key = Engine::Key::A;     actions.AddBinding(b);
	b.action = m_move_right; b.key = Engine::Key::D;     actions.AddBinding(b);
	b.action = m_jump;       b.key = Engine::Key::Space; actions.AddBinding(b);

	auto& contexts = App().GetInput().GetContexts();
	auto ctx = contexts.RegisterContext("Game", 0, Engine::InputConsumptionPolicy::Consume);
	contexts.PushContext(ctx);

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

	if (input.WasActionPressed(m_jump))
	{
		LOG_INFO("Jump pressed!");
	}
}

void PlatformerGame::OnShutdown()
{
	LOG_INFO("Game shutdown");
}
