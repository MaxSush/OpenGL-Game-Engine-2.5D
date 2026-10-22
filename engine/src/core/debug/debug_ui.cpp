#include <engine/core/debug/debug_ui.h>

#include <imgui.h>

namespace Engine
{
	bool DebugUI::Begin(const char* title)
	{
		return ImGui::Begin(title);
	}

	void DebugUI::End()
	{
		ImGui::End();
	}

	void DebugUI::Text(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		ImGui::TextV(fmt, args);
		va_end(args);
	}

	void DebugUI::Seperator()
	{
		ImGui::Separator();
	}

	bool DebugUI::Button(const char* label)
	{
		return ImGui::Button(label);
	}

	bool DebugUI::Checkbox(const char* label, bool& value)
	{
		return ImGui::Checkbox(label, &value);
	}

	bool DebugUI::SliderFloat(const char* label, float& value, float min, float max)
	{
		return ImGui::SliderFloat(label, &value, min, max);
	}
}