#pragma once

#include <engine/engine_api.h>

namespace Engine
{
	class Application;
	class DebugUI;

	class ENGINE_API Game
	{
	public:
		Game() = default;
		virtual ~Game() = default;

		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;

		// Register actions/contexts here. Return false to abort startup.
		// Do NOT call Input::FinalizeActionSetup(): Application does it after OnStart returns true.
		virtual bool OnStart() { return true; }

		// Once per rendered frame, after input is updated. Sample input "intent" here.
		virtual void OnUpdate(float /*dt*/) {}

		// Zero or more times per frame at a constant step. Put physics/movement here.
		virtual void OnFixedUpdate(float /*step*/) {}

		// After the framebuffer is cleared, before the buffer swap.
		virtual void OnRender() {}

		// Called once per rendered frame when AppConfig::enable_debug_ui is true.
		virtual void OnDebugUI(DebugUI& /* ui */) {}

		// Framebuffer size changed (never called with 0x0).
		virtual void OnResize(int /*width*/, int /*height*/) {}

		virtual void OnShutdown() {}

	protected:
		Application& App() { return *m_app; }

	private:
		friend class Application;
		Application* m_app = nullptr;
	};
}