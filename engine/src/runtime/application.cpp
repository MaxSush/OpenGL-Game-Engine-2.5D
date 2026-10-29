#include <engine/application.h>
#include <engine/game.h>
#include <engine/core/logger/log.h>
#include <engine/input/input.h>
#include <engine/renderer/renderer.h>

#include <core/debug/imgui_layer.h>
#include <platform/glfw/glfw_window.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <exception>
#include <thread>

namespace Engine
{
	Application::Application(Game& game, const AppConfig& config)
		: m_game(game),
		m_config(config)
	{
		m_time.fixed_step = m_config.fixed_step;
	}

	Application::~Application() = default;

	void Application::RequestQuit()
	{
		if (m_window)
		{
			m_window->RequestClose();
		}
	}

	void Application::OnFramebufferResize(int width, int height)
	{
		if (width <= 0 || height <= 0)
		{
			return;
		}
		if (m_renderer)
		{
			m_renderer->SetViewport(width, height);
		}
		m_game.OnResize(width, height);
	}

	void Application::UpdateFps(float dt)
	{
		m_fps_timer += dt;
		++m_fps_frame;
		if (m_fps_timer < 0.5f)
		{
			return;
		}

		m_time.fps = static_cast<float>(m_fps_frame) / m_fps_timer;
		m_fps_timer = 0.0f;
		m_fps_frame = 0;

		if (m_config.show_fps_in_title)
		{
			char buf[64];
			std::snprintf(buf, sizeof(buf), " | %.0f FPS", m_time.fps);
			m_window->SetTitle(m_config.window.title + buf);
		}
	}

	int Application::Run()
	{
		m_window = Window::Create(m_config.window);
		m_input = std::make_unique<Input>(*m_window);
		m_renderer = std::unique_ptr<Renderer>(new Renderer());
		m_world = std::make_unique<World>();

		if (m_config.enable_debug_ui)
		{
			auto* glfw_window = dynamic_cast<GLFW_Window*>(m_window.get());
			if (!glfw_window)
			{
				throw std::runtime_error("Debug UI requires a GLFW window backend");
			}
			m_imgui_layer = std::make_unique<ImGuiLayer>(glfw_window->GetNativeHandle());
		}

		m_window->SetResizeCallback([this](int w, int h) { OnFramebufferResize(w, h); });

		m_game.m_app = this;
		struct GameBinding
		{
			Game& game;
			~GameBinding()
			{
				game.m_app = nullptr;
			}
		} binding{ m_game };

		const Dimension fb = m_window->GetFramebufferSize();
		m_renderer->SetViewport(fb.width, fb.height);

		bool started = false;
		try
		{
			started = m_game.OnStart();
		}
		catch (const std::exception& e)
		{
			LOG_ERROR("Application::Run => Game::OnStart threw: {}", e.what());
		}
		catch (...)
		{
			LOG_ERROR("Application::Run => Game::OnStart threw an unknown exception");
		}

		if (!started)
		{
			LOG_ERROR("Application::Run => Game::OnStart returned false");
			return 1;
		}

		auto shutdown_game = [this]
			{
				m_game.OnShutdown();
				m_world->Clear();
			};

		if (!m_input->FinalizeActionSetup())
		{
			LOG_ERROR("Application::Run => FinalizeActionSetup failed (did the game call it already?)");
			shutdown_game();
			return 1;
		}

		using Clock = std::chrono::steady_clock;
		auto last = Clock::now();
		float accumulator = 0.0f;

		try
		{

			const Dimension size = m_window->GetFramebufferSize();
			OnFramebufferResize(size.width, size.height);

			m_world->Flush(); // commands deferred in OnStart take effect before frame 1

			while (!m_window->ShouldClose())
			{
				const auto now = Clock::now();
				float dt = std::chrono::duration<float>(now - last).count();
				last = now;
				dt = std::min(dt, m_config.max_frame_time);

				m_window->PollEvents();
				m_input->BeginFrame();

				if (m_window->IsMinimized())
				{
					m_input->EndFrame();
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					last = Clock::now();
					continue;
				}

				m_time.delta = dt;
				m_time.total += dt;
				++m_time.frame;

				m_game.OnUpdate(dt);
				m_world->Flush();

				accumulator += dt;
				int steps = 0;
				while (accumulator >= m_config.fixed_step && steps < m_config.max_fixed_steps_per_frame)
				{
					m_game.OnFixedUpdate(m_config.fixed_step);
					m_world->Flush();
					accumulator -= m_config.fixed_step;
					++steps;
				}

				if (accumulator >= m_config.fixed_step)
				{
					accumulator = 0.0f;
				}

				m_renderer->BeginFrame(m_config.clear_color);
				m_game.OnRender();
				if (m_imgui_layer)
				{
					m_imgui_layer->BeginFrame();
					m_game.OnDebugUI(m_debug_ui);
					m_imgui_layer->EndFrame();
				}
				m_window->SwapBuffers();

				m_input->EndFrame();
				UpdateFps(dt);
			}
		}
		catch (...)
		{
			shutdown_game();
			throw;
		}

		shutdown_game();
		return 0;
	}

	int Run(Game& game, const AppConfig& config)
	{
		try
		{
			Log::Init();
		}
		catch (const std::exception& e)
		{
			std::fprintf(stderr, "Logger init failed: %s\n", e.what());
			return 1;
		}

		int code = 1;
		try
		{
			Application app(game, config);
			code = app.Run();
		}
		catch (const std::exception& e)
		{
			LOG_CRITICAL("Unhandled exception: {}", e.what());
		}
		catch (...)
		{
			LOG_CRITICAL("Unknown unhandled exception");
		}

		Log::Shutdown();
		return code;
	}
}