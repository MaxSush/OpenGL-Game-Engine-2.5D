#pragma once

#include <engine/engine_api.h>

namespace Engine
{
	class Application;

	class ENGINE_API DebugUI
	{
	public:
		bool Begin(const char* title);
		void End();

		void Text(const char* fmt, ...);
		void Seperator();
		bool Button(const char* label);
		bool Checkbox(const char* lable, bool& value);
		bool SliderFloat(const char* label, float& value, float min, float max);

	private:
		friend class Application;

		DebugUI() = default;
	};

}