#include "imgui_layer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace Engine
{
	ImGuiLayer::ImGuiLayer(GLFWwindow* window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
		{
			ImGui::DestroyContext();
			throw std::runtime_error("Failed to initialize ImGui GLFW backend");
		}
		m_glfw_initialized = true;

		if (!ImGui_ImplOpenGL3_Init("#version 330 core"))
		{
			ImGui_ImplGlfw_Shutdown();
			m_glfw_initialized = false;
			ImGui::DestroyContext();
			throw std::runtime_error("Failed to initialize ImGui OpenGL3 backend");
		}
		m_opengl_initialized = true;

		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;
	}

	ImGuiLayer::~ImGuiLayer()
	{
		if (m_opengl_initialized)
		{
			ImGui_ImplOpenGL3_Shutdown();
		}
		if (m_glfw_initialized)
		{
			ImGui_ImplGlfw_Shutdown();
		}
		if (m_glfw_initialized || m_opengl_initialized)
		{
			ImGui::DestroyContext();
		}
	}

	void ImGuiLayer::BeginFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::EndFrame()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}