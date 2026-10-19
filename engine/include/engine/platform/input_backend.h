#pragma once

#include <engine/input/input_types.h>

namespace Engine
{
	// Forward Declared
	class InputEventQueue;

	class InputBackend
	{
	public:
		virtual ~InputBackend() = default;

		virtual void SetEventQueue(InputEventQueue* queue) = 0;
		virtual void SetCursorMode(CursorMode mode) = 0;
		virtual CursorMode GetCursorMode() const = 0;
	};
}