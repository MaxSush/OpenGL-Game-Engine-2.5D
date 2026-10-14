#include <engine/core/logger/log.h>
#include <engine/platform/platform_window.h>
#include <engine/input/input.h>

int main()
{
	Engine::Log::Init();
	LOG_INFO("Game Started");

	auto wnd = Engine::Window::Create();
	Engine::Input input(*wnd);

	auto move_left = input.GetActionMap().RegisterAction("MoveLeft", Engine::InputActionType::Digital);
	auto move_right = input.GetActionMap().RegisterAction("MoveRight", Engine::InputActionType::Digital);
	auto jump = input.GetActionMap().RegisterAction("Jump", Engine::InputActionType::Digital);

	Engine::InputBinding b;
	b.action = move_left;
	b.source = Engine::InputBindingSource::Key;
	b.key = Engine::Key::A;
	input.GetActionMap().AddBinding(b);

	b.action = move_right;
	b.key = Engine::Key::D;
	input.GetActionMap().AddBinding(b);

	b.action = jump;
	b.key = Engine::Key::Space;
	input.GetActionMap().AddBinding(b);

	if (!input.FinalizeActionSetup())
	{
		LOG_ERROR("FinalizeActionSetup initialization failure and stopped setup");
		return 1;
	}

	auto game_ctx = input.GetContexts().RegisterContext("Game", 0, Engine::InputConsumptionPolicy::Consume);
	input.GetContexts().PushContext(game_ctx);

	while (!wnd->ShouldClose())
	{
		wnd->PollEvents();
		input.BeginFrame();

		if (input.WasKeyPressed(Engine::Key::A))
		{
			LOG_INFO("A pressed");
		}
		if (input.IsKeyDown(Engine::Key::S))
		{
			LOG_INFO("S Down");
		}
		if (input.WasMouseButtonPressed(Engine::MouseButton::Left))
		{
			LOG_INFO("Mouse left pressed");
		}

		if (input.WasActionPressed(jump))
		{
			LOG_INFO("Jump pressed!");
		}

		if (input.IsActionDown(move_left))
		{
			LOG_INFO("Moving left");
		}
		else if (input.IsActionDown(move_right))
		{
			LOG_INFO("Moving right");
		}

		wnd->SwapBuffers();
		input.EndFrame();
	}

	Engine::Log::Shutdown();

	return 0;
}