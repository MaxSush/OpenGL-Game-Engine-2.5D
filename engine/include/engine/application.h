#pragma once

#include <engine/core/debug/debug_ui.h>
#include <engine/engine_api.h>
#include <engine/platform/platform_window.h>
#include <engine/core/time.h>

#include <memory>
#include <array>

namespace Engine
{
	class Game;
	class Input;
	class Renderer;
	class ImGuiLayer;

	struct AppConfig
	{
		WindowProps window;
		float fixed_step = 1.0f / 60.0f;
		float max_frame_time = 0.25f;
		int max_fixed_steps_per_frame = 8;
		bool show_fps_in_title = true;
		bool enable_debug_ui = true;
		std::array<float, 4> clear_color{ 0.10f,0.10f,0.12f,1.0f };
	};

	class ENGINE_API Application
	{
	public:
		Application(Game& game, const AppConfig& config);
		~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		int Run();
		void RequestQuit();

		Window& GetWindow() { return *m_window; }
		Input& GetInput() { return *m_input; }
		Renderer& GetRenderer() { return *m_renderer; }
		const FrameTime& GetTime() const { return m_time; }

	private:
		void OnFramebufferResize(int width, int height);
		void UpdateFps(float dt);

		Game& m_game;
		AppConfig m_config;
		std::unique_ptr<Window> m_window;
		std::unique_ptr<Input> m_input;
		std::unique_ptr<Renderer> m_renderer;
		std::unique_ptr<ImGuiLayer> m_imgui_layer;
		DebugUI m_debug_ui;
		FrameTime m_time;

		float m_fps_timer = 0.0f;
		int m_fps_frame = 0;
	};

	// The one entry point for games. Owns logger init/shutdown and exception reporting.
	ENGINE_API int Run(Game& game, const AppConfig& config = {});
}