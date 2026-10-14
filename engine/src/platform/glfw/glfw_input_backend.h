#pragma once

#include <engine/platform/input_backend.h>
#include "input/input_event_queue.h"

struct GLFWwindow;

namespace Engine
{
	class GLFWInputBackend : public InputBackend
	{
	public:
		explicit GLFWInputBackend(GLFWwindow* window);
		~GLFWInputBackend() override;

		// Inherited via InputBackend
		void SetEventQueue(InputEventQueue* queue) override;
		void SetCursorMode(CursorMode mode) override;
		CursorMode GetCursorMode() const override;
		void PumpEvents() override;

	private:
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
		static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void WindowFocusCallback(GLFWwindow* window, int focused);
		static void CursorEnterCallback(GLFWwindow* window, int entered);
		static void CharCallback(GLFWwindow* window, unsigned int codepoint);

		GLFWwindow* m_window = nullptr;
		InputEventQueue* m_queue = nullptr;
		CursorMode m_cursor_mode = CursorMode::Visible;
	};
}