#pragma once

#include <engine/platform/platform_window.h>

struct GLFWwindow;

namespace Engine
{
	class GLFWInputBackend;

	class GLFW_Window : public Window
	{
	public:
		explicit GLFW_Window(const WindowProps& props);
		~GLFW_Window() override;

		// Inherited via Window
		void PollEvents() override;
		bool ShouldClose() const override;
		void SwapBuffers() override;
		Dimension GetSize() const override;
		Dimension GetFramebufferSize() const override;
		void SetTitle(std::string_view title) override;
		void SetVSync(bool enabled) override;
		void RequestClose() override;
		bool IsMinimized() const override;
		void SetResizeCallback(ResizeCallback callback) override;
		std::unique_ptr<InputBackend> CreateInputBackend() override;

		GLFWwindow* GetNativeHandle() const { return m_window; }
		void AttachInputBackend(GLFWInputBackend* backend);
		void DetachInputBackend(GLFWInputBackend* backend);

	private:
		static void ErrorCallback(int error, const char* msg);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
		static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void WindowFocusCallback(GLFWwindow* window, int focused);
		static void CursorEnterCallback(GLFWwindow* window, int entered);
		static void CharCallback(GLFWwindow* window, unsigned int codepoint);
		static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

		ResizeCallback m_resize_callback;
		GLFWwindow* m_window = nullptr;
		GLFWInputBackend* m_input_backend = nullptr;
	};
}