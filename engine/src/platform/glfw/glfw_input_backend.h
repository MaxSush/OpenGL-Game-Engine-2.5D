#pragma once

#include <engine/platform/input_backend.h>
#include "input/input_event_queue.h"

struct GLFWwindow;

namespace Engine
{
	class GLFW_Window;

	class GLFWInputBackend : public InputBackend
	{
	public:
		explicit GLFWInputBackend(GLFW_Window* owner);
		~GLFWInputBackend() override;

		// Inherited via InputBackend
		void SetEventQueue(InputEventQueue* queue) override;
		void SetCursorMode(CursorMode mode) override;
		CursorMode GetCursorMode() const override;

		// Called by GLFW_Window
		void OnWindowDestroyed();
		void OnKey(int key, int scancode, int action, int mods);
		void OnMouseButton(int button, int action, int mods);
		void OnCursorPos(double xpos, double ypos);
		void OnScroll(double xoffset, double yoffset);
		void OnWindowFocus(int focused);
		void OnCursorEnter(int entered);
		void OnChar(unsigned int codepoint);

	private:
		GLFW_Window* m_owner = nullptr;
		GLFWwindow* m_window = nullptr;
		InputEventQueue* m_queue = nullptr;
		CursorMode m_cursor_mode = CursorMode::Visible;
	};
}