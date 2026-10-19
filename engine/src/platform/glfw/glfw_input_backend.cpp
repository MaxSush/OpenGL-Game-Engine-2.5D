#include "glfw_input_backend.h"
#include "glfw_window.h"

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

	GLFWInputBackend::GLFWInputBackend(GLFW_Window* owner)
		: m_owner(owner)
	{
		if (!m_owner) return;
		m_window = m_owner->GetNativeHandle();
		m_owner->AttachInputBackend(this);
	}

	GLFWInputBackend::~GLFWInputBackend()
	{
		glfwSetKeyCallback(m_window, nullptr);
		glfwSetMouseButtonCallback(m_window, nullptr);
		glfwSetCursorPosCallback(m_window, nullptr);
		glfwSetScrollCallback(m_window, nullptr);
		glfwSetWindowFocusCallback(m_window, nullptr);
		glfwSetCursorEnterCallback(m_window, nullptr);
		glfwSetCharCallback(m_window, nullptr);

		glfwSetWindowUserPointer(m_window, nullptr);

		if (m_owner)
		{
			m_owner->DetachInputBackend(this);
		}
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

	void GLFWInputBackend::OnWindowDestroyed()
	{
		m_owner = nullptr;
		m_window = nullptr;
	}

	void GLFWInputBackend::OnKey(int key, int, int action, int mods)
	{
		if (!m_queue)
		{
			return;
		}

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
		m_queue->Push(event);
	}

	void GLFWInputBackend::OnMouseButton(int button, int action, int mods)
	{
		if (!m_queue)
		{
			return;
		}

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
		m_queue->Push(event);
	}

	void GLFWInputBackend::OnCursorPos(double xpos, double ypos)
	{
		if (!m_queue)
		{
			return;
		}

		InputEvent event;
		event.type = InputEventType::MouseMove;
		event.payload = MouseMoveEvent{ xpos, ypos };
		m_queue->Push(event);

	}

	void GLFWInputBackend::OnScroll(double xoffset, double yoffset)
	{
		if (!m_queue)
		{
			return;
		}

		InputEvent event;
		event.type = InputEventType::MouseScroll;
		event.payload = MouseScrollEvent{ xoffset, yoffset };
		m_queue->Push(event);
	}

	void GLFWInputBackend::OnWindowFocus(int focused)
	{
		if (!m_queue)
		{
			return;
		}

		InputEvent event;
		event.type = focused ? InputEventType::WindowFocusGained : InputEventType::WindowFocusLost;
		m_queue->Push(event);
	}

	void GLFWInputBackend::OnCursorEnter(int entered)
	{
		if (!m_queue)
		{
			return;
		}

		InputEvent event;
		event.type = entered ? InputEventType::CursorEnter : InputEventType::CursorLeave;
		m_queue->Push(event);
	}

	void GLFWInputBackend::OnChar(unsigned int codepoint)
	{
		if (!m_queue)
		{
			return;
		}

		InputEvent event;
		event.type = InputEventType::TextInput;
		event.payload = TextInputEvent{ codepoint };
		m_queue->Push(event);
	}
}