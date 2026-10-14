#pragma once

#include <engine/engine_api.h>
#include <engine/input/input_types.h>

#include <vector>

namespace Engine
{
	struct InputFrame
	{
		uint64_t frame_id = 0;

		bool KeyDown(Key key) const;
		bool KeyPressed(Key key) const;
		bool KeyReleased(Key key) const;

		bool MouseButtonDown(MouseButton button) const;
		bool MouseButtonPressed(MouseButton button) const;
		bool MouseButtonReleased(MouseButton button) const;

		InputVector2 GetMousePosition() const;
		InputVector2 GetMouseDelta() const;
		InputVector2 GetScrollDelta() const;

		bool IsActionDown(InputActionId action) const;
		bool WasActionPressed(InputActionId action) const;
		bool WasActionReleased(InputActionId action) const;

		float GetAxis(InputActionId action) const;
		InputVector2 GetVector2(InputActionId action) const;

		struct ActionState
		{
			bool down = false;
			bool pressed = false;
			bool released = false;
			float scalar = 0.0f;
			InputVector2 vector2{};
		};

		struct PhysicalKeyState
		{
			bool down = false;
			bool pressed = false;
			bool released = false;
		};

		struct PhysicalMouseState
		{
			bool down = false;
			bool pressed = false;
			bool released = false;
		};

		PhysicalKeyState key_states[static_cast<int>(Key::Count)]{};
		PhysicalMouseState mouse_states[static_cast<int>(MouseButton::Count)]{};
		InputVector2 mouse_position{};
		InputVector2 mouse_delta{};
		InputVector2 scroll_delta{};

		std::vector<ActionState> action_states;
	};
}