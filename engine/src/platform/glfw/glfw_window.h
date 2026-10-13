#pragma once

#include <engine/platform/platform_window.h>

struct GLFWwindow;

namespace Engine
{
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

	private:
		static void ErrorCallback(int error, const char* msg);

		GLFWwindow* m_window = nullptr;
	};
}