#include "glfw_input_backend.h"

#include <GLFW/glfw3.h>

namespace Engine
{
	namespace
	{
		Key GlfwToKey(int key)
		{
			if (key < 0 || key >= static_cast<int>(Key::Count))
			{
				return Key::Unknown;
			}
			return static_cast<Key>(key);
		}

		MouseButton GlfwToMouseButton(int button)
		{
			if (button < 0 || button >= static_cast<int>(MouseButton::Count))
			{
				return MouseButton::None;
			}
			return static_cast<MouseButton>(button);
		}

		ModifierKey GlfwToMods(int mods)
		{
			ModifierKey result = ModifierKey::None;
			if (mods & GLFW_MOD_SHIFT) 
			{
				result = result | ModifierKey::Shift;
			}
			if (mods & GLFW_MOD_CONTROL) 
			{
				result = result | ModifierKey::Control;
			}
			if (mods & GLFW_MOD_ALT) 
			{
				result = result | ModifierKey::Alt;
			}
			if (mods & GLFW_MOD_SUPER) 
			{
				result = result | ModifierKey::Super;
			}
			if (mods & GLFW_MOD_CAPS_LOCK) 
			{
				result = result | ModifierKey::CapsLock;
			}
			if (mods & GLFW_MOD_NUM_LOCK) 
			{
				result = result | ModifierKey::NumLock;
			}
			return result;
		}
	}

	GLFWInputBackend::GLFWInputBackend(GLFWwindow* window)
		: m_window(window)
	{
		if (!m_window) return;

		glfwSetWindowUserPointer(m_window, this);

		glfwSetKeyCallback(m_window, &GLFWInputBackend::KeyCallback);
		glfwSetMouseButtonCallback(m_window, &GLFWInputBackend::MouseButtonCallback);
		glfwSetCursorPosCallback(m_window, &GLFWInputBackend::CursorPosCallback);
		glfwSetScrollCallback(m_window, &GLFWInputBackend::ScrollCallback);
		glfwSetWindowFocusCallback(m_window, &GLFWInputBackend::WindowFocusCallback);
		glfwSetCursorEnterCallback(m_window, &GLFWInputBackend::CursorEnterCallback);
		glfwSetCharCallback(m_window, &GLFWInputBackend::CharCallback);
	}

	GLFWInputBackend::~GLFWInputBackend()
	{
		if (!m_window) return;

		glfwSetKeyCallback(m_window, nullptr);
		glfwSetMouseButtonCallback(m_window, nullptr);
		glfwSetCursorPosCallback(m_window, nullptr);
		glfwSetScrollCallback(m_window, nullptr);
		glfwSetWindowFocusCallback(m_window, nullptr);
		glfwSetCursorEnterCallback(m_window, nullptr);
		glfwSetCharCallback(m_window, nullptr);

		glfwSetWindowUserPointer(m_window, nullptr);
	}

	void GLFWInputBackend::SetEventQueue(InputEventQueue* queue)
	{
		m_queue = queue;
	}

	void GLFWInputBackend::SetCursorMode(CursorMode mode)
	{
		m_cursor_mode = mode;
		if (!m_window) return;

		int glfw_mode = GLFW_CURSOR_NORMAL;
		switch (mode)
		{
		case CursorMode::Visible:
			glfw_mode = GLFW_CURSOR_NORMAL;
			break;
		case CursorMode::Hidden:
			glfw_mode = GLFW_CURSOR_HIDDEN;
			break;
		case CursorMode::Captured:
			glfw_mode = GLFW_CURSOR_DISABLED;
			break;
		}
		glfwSetInputMode(m_window, GLFW_CURSOR, glfw_mode);
	}

	CursorMode GLFWInputBackend::GetCursorMode() const
	{
		return m_cursor_mode;
	}

	void GLFWInputBackend::PumpEvents()
	{
		glfwPollEvents();
	}

	void GLFWInputBackend::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto* self = static_cast<GLFWInputBackend*>(glfwGetWindowUserPointer(window));
		if (!self || !self->m_queue) return;

		InputEvent event;
		KeyEvent ke;
		ke.key = GlfwToKey(key);
		ke.mods = GlfwToMods(mods);

		switch (action)
		{
		case GLFW_PRESS:
			event.type = InputEventType::KeyDown;
			break;
		case GLFW_RELEASE:
			event.type = InputEventType::KeyUp;
			break;
		case GLFW_REPEAT:
			event.type = InputEventType::KeyRepeat;
			break;
		default:
			return;
		}

		event.payload = ke;
		self->m_queue->Push(event);
	}

	void GLFWInputBackend::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		auto* self = static_cast<GLFWInputBackend*>(glfwGetWindowUserPointer(window));
		if (!self || !self->m_queue) return;

		InputEvent event;
		MouseButtonEvent me;
		me.button = GlfwToMouseButton(button);
		me.mods = GlfwToMods(mods);

		if (action == GLFW_PRESS)
			event.type = InputEventType::MouseButtonDown;
		else if (action == GLFW_RELEASE)
			event.type = InputEventType::MouseButtonUp;
		else
			return;

		event.payload = me;
		self->m_queue->Push(event);
	}

	void GLFWInputBackend::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto* self = static_cast<GLFWInputBackend*>(glfwGetWindowUserPointer(window));
		if (!self || !self->m_queue) return;

		InputEvent event;
		event.type = InputEventType::MouseMove;
		MouseMoveEvent me;
		me.xpos = xpos;
		me.ypos = ypos;
		event.payload = me;
		self->m_queue->Push(event);
	}

	void GLFWInputBackend::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto* self = static_cast<GLFWInputBackend*>(glfwGetWindowUserPointer(window));
		if (!self || !self->m_queue) return;

		InputEvent event;
		event.type = InputEventType::MouseScroll;
		MouseScrollEvent se;
		se.xoffset = xoffset;
		se.yoffset = yoffset;
		event.payload = se;
		self->m_queue->Push(event);
	}

	void GLFWInputBackend::WindowFocusCallback(GLFWwindow* window, int focused)
	{
		auto* self = static_cast<GLFWInputBackend*>(glfwGetWindowUserPointer(window));
		if (!self || !self->m_queue) return;

		InputEvent event;
		event.type = (focused) ? InputEventType::WindowFocusGained : InputEventType::WindowFocusLost;
		self->m_queue->Push(event);
	}

	void GLFWInputBackend::CursorEnterCallback(GLFWwindow* window, int entered)
	{
		auto* self = static_cast<GLFWInputBackend*>(glfwGetWindowUserPointer(window));
		if (!self || !self->m_queue) return;

		InputEvent event;
		event.type = (entered) ? InputEventType::CursorEnter : InputEventType::CursorLeave;
		self->m_queue->Push(event);
	}

	void GLFWInputBackend::CharCallback(GLFWwindow* window, unsigned int codepoint)
	{
		auto* self = static_cast<GLFWInputBackend*>(glfwGetWindowUserPointer(window));
		if (!self || !self->m_queue) return;

		InputEvent event;
		event.type = InputEventType::TextInput;
		TextInputEvent te;
		te.codepoint = codepoint;
		event.payload = te;
		self->m_queue->Push(event);
	}
}