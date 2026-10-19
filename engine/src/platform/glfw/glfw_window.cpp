#include "glfw_window.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <engine/core/logger/log.h>
#include <engine/platform/input_backend.h>
#include "glfw_input_backend.h"

namespace Engine
{
	std::unique_ptr<Window> Window::Create(const WindowProps& props)
	{
		return std::make_unique<GLFW_Window>(props);
	}

	Engine::GLFW_Window::GLFW_Window(const WindowProps& props)
	{
		glfwSetErrorCallback(GLFW_Window::ErrorCallback);

		if (!glfwInit())
		{
			throw WindowException("GLFWWindow::GLFWWindow => glfwInit failed");
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
			throw WindowException("GLFWWindow::GLFWWindow => failed to initialize OpenGL context");
		}

		glfwSetWindowUserPointer(m_window, this);
		glfwSetKeyCallback(m_window, &GLFW_Window::KeyCallback);
		glfwSetMouseButtonCallback(m_window, &GLFW_Window::MouseButtonCallback);
		glfwSetCursorPosCallback(m_window, &GLFW_Window::CursorPosCallback);
		glfwSetScrollCallback(m_window, &GLFW_Window::ScrollCallback);
		glfwSetWindowFocusCallback(m_window, &GLFW_Window::WindowFocusCallback);
		glfwSetCursorEnterCallback(m_window, &GLFW_Window::CursorEnterCallback);
		glfwSetCharCallback(m_window, &GLFW_Window::CharCallback);
		glfwSetFramebufferSizeCallback(m_window, &GLFW_Window::FramebufferSizeCallback);

		glfwSwapInterval((props.vsync) ? 1 : 0);

		LOG_INFO("Graphics Card Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		LOG_INFO("Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		LOG_INFO("OpenGL Version: {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
	}

	GLFW_Window::~GLFW_Window()
	{
		if (m_input_backend)
		{
			m_input_backend->OnWindowDestroyed();
			m_input_backend = nullptr;
		}
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

	void GLFW_Window::RequestClose()
	{
		glfwSetWindowShouldClose(m_window, GLFW_TRUE);
	}

	bool GLFW_Window::IsMinimized() const
	{
		return (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) == GLFW_TRUE);
	}

	void GLFW_Window::SetResizeCallback(ResizeCallback callback)
	{
		m_resize_callback = std::move(callback);
	}

	std::unique_ptr<InputBackend> GLFW_Window::CreateInputBackend()
	{
		return std::make_unique<GLFWInputBackend>(this);
	}

	void GLFW_Window::AttachInputBackend(GLFWInputBackend* backend)
	{
		m_input_backend = backend;
	}

	void GLFW_Window::DetachInputBackend(GLFWInputBackend* backend)
	{
		if (m_input_backend == backend)
		{
			m_input_backend = nullptr;
		}
	}

	void GLFW_Window::ErrorCallback(int error, const char* msg)
	{
		LOG_ERROR("GLFWWindow::ErrorCallback => [{}] : {}", error, msg);
	}

#define ENGINE_GLFW_SELF()	auto* self = static_cast<GLFW_Window*>(glfwGetWindowUserPointer(window)); if(!self  || !self->m_input_backend) return;

	void GLFW_Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		ENGINE_GLFW_SELF();
		self->m_input_backend->OnKey(key, scancode, action, mods);
	}

	void GLFW_Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		ENGINE_GLFW_SELF();
		self->m_input_backend->OnMouseButton(button, action, mods);
	}

	void GLFW_Window::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		ENGINE_GLFW_SELF();
		self->m_input_backend->OnCursorPos(xpos, ypos);
	}

	void GLFW_Window::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		ENGINE_GLFW_SELF();
		self->m_input_backend->OnScroll(xoffset, yoffset);
	}

	void GLFW_Window::WindowFocusCallback(GLFWwindow* window, int focused)
	{
		ENGINE_GLFW_SELF();
		self->m_input_backend->OnWindowFocus(focused);
	}

	void GLFW_Window::CursorEnterCallback(GLFWwindow* window, int entered)
	{
		ENGINE_GLFW_SELF();
		self->m_input_backend->OnCursorEnter(entered);
	}

	void GLFW_Window::CharCallback(GLFWwindow* window, unsigned int codepoint)
	{
		ENGINE_GLFW_SELF();
		self->m_input_backend->OnChar(codepoint);
	}
#undef ENGINE_GLFW_SELF

	void GLFW_Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
	{
		auto* self = static_cast<GLFW_Window*>(glfwGetWindowUserPointer(window));
		if (self && self->m_resize_callback)
		{
			self->m_resize_callback(width, height);
		}
	}
}