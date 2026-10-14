#pragma once

#include <engine/engine_api.h>

#include <string>
#include <string_view>
#include <stdexcept>
#include <memory>

namespace Engine
{
	// Forward Declared
	class InputBackend;

	struct Dimension
	{
		int width = 0;
		int height = 0;
	};

	// Required Configuration while Initializing Window
	struct WindowProps
	{
		int width					= 1280;
		int height					= 720;
		std::string title			= "OpenGL Engine";
		bool resizable				= false;
		bool vsync					= true;
		int openGL_version_major	= 3;
		int openGL_version_minor	= 3;
		int depth_bits				= 24;	// request depth buffer
		int samples					= 0;	// 0 = MSAA off
	};

# pragma warning (push) 
# pragma warning (disable: 4275)
	class ENGINE_API WindowException : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};
# pragma warning (pop)

	class ENGINE_API Window
	{
	public:
		static std::unique_ptr<Window> Create(const WindowProps& props = {});
		virtual ~Window() = default;

		Window(const Window&) = delete;
		Window& operator =(const Window&) = delete; 
		Window(Window&&) = delete;
		Window& operator =(Window&&) = delete;

		virtual void PollEvents() = 0;
		virtual bool ShouldClose() const = 0;
		virtual void SwapBuffers() = 0;

		virtual Dimension GetSize() const = 0;
		virtual Dimension GetFramebufferSize() const = 0;

		virtual void SetTitle(std::string_view title) = 0;
		virtual void SetVSync(bool enabled) = 0;

		virtual std::unique_ptr<InputBackend> CreateInputBackend() = 0;

	protected:
		Window() = default;

	private:

	};

}
