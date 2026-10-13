#include "glfw_window.h"

#include <engine/core/logger/log.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Engine
{
	std::unique_ptr<Window> Window::Create(const WindowProps& props)
	{
		return std::make_unique<GLFW_Window>(props);
	}

	Engine::GLFW_Window::GLFW_Window(const WindowProps& props)
	{
		glfwSetErrorCallback(GLFW_Window::ErrorCallback);

		int err = glfwInit();
		if (!err)
		{
			throw WindowException(std::format("GLFWWindow::GLFWWindow => [{}] glfwInit failed", err));
		}
			
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, props.openGL_version_major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, props.openGL_version_minor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, (props.resizable) ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_DEPTH_BITS, props.depth_bits);
		glfwWindowHint(GLFW_SAMPLES, props.samples);

		m_window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);
		if (!m_window)
		{
			glfwTerminate();
			throw WindowException("GLFWWindow::GLFWWindow => create window failed");
		}
		glfwMakeContextCurrent(m_window);

		int version = gladLoadGL(reinterpret_cast <GLADloadfunc>(glfwGetProcAddress));
		if (version == 0) 
		{
			glfwDestroyWindow(m_window);
			glfwTerminate();
			throw WindowException(std::format("GLFWWindow::GLFWWindow => [{}] failed to initialize OpenGL context", version));
		}

		glfwSwapInterval((props.vsync) ? 1 : 0);

		LOG_INFO("Graphics Card Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		LOG_INFO("Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		LOG_INFO("OpenGL Version: {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
	}

	GLFW_Window::~GLFW_Window()
	{
		if (m_window)
		{
			glfwDestroyWindow(m_window);
		}
		glfwTerminate();
	}

	void GLFW_Window::PollEvents()
	{
		glfwPollEvents();
	}

	bool GLFW_Window::ShouldClose() const
	{
		return glfwWindowShouldClose(m_window) != 0;
	}

	void GLFW_Window::SwapBuffers()
	{
		glfwSwapBuffers(m_window);
	}

	Dimension GLFW_Window::GetSize() const
	{
		Dimension d{};
		glfwGetWindowSize(m_window, &d.width, &d.height);
		return d;
	}

	Dimension GLFW_Window::GetFramebufferSize() const
	{
		Dimension d{};
		glfwGetFramebufferSize(m_window, &d.width, &d.height);
		return d;
	}

	void GLFW_Window::SetTitle(std::string_view title)
	{
		glfwSetWindowTitle(m_window, std::string(title).c_str());
	}

	void GLFW_Window::SetVSync(bool enabled)
	{
		glfwSwapInterval((enabled) ? 1 : 0);
	}

	void GLFW_Window::ErrorCallback(int error, const char* msg)
	{
		LOG_ERROR("GLFWWindow::ErrorCallback => [{}] : {}", error, msg);
	}
}