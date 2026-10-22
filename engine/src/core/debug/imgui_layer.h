#pragma once

struct GLFWwindow;

namespace Engine
{
	class ImGuiLayer
	{
	public:
		explicit ImGuiLayer(GLFWwindow* window);
		~ImGuiLayer();

		ImGuiLayer(const ImGuiLayer&) = delete;
		ImGuiLayer& operator=(const ImGuiLayer&) = delete;

		void BeginFrame();
		void EndFrame();

	private:
		bool m_glfw_initialized = false;
		bool m_opengl_initialized = false;
	};
}