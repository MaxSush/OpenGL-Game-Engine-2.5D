#include <engine/input/input_frame.h>

namespace Engine
{
	bool InputFrame::KeyDown(Key key) const
	{
		int idx = static_cast<int>(key);
		if (idx < 0 || idx >= static_cast<int>(Key::Count)) 
		{
			return false;
		}
		return key_states[idx].down;
	}

	bool InputFrame::KeyPressed(Key key) const
	{
		int idx = static_cast<int>(key);
		if (idx < 0 || idx >= static_cast<int>(Key::Count)) 
		{
			return false;
		}
		return key_states[idx].pressed;
	}

	bool InputFrame::KeyReleased(Key key) const
	{
		int idx = static_cast<int>(key);
		if (idx < 0 || idx >= static_cast<int>(Key::Count)) 
		{
			return false;
		}
		return key_states[idx].released;
	}

	bool InputFrame::MouseButtonDown(MouseButton button) const
	{
		int idx = static_cast<int>(button);
		if (idx < 0 || idx >= static_cast<int>(MouseButton::Count)) 
		{
			return false;
		}
		return mouse_states[idx].down;
	}

	bool InputFrame::MouseButtonPressed(MouseButton button) const
	{
		int idx = static_cast<int>(button);
		if (idx < 0 || idx >= static_cast<int>(MouseButton::Count)) 
		{
			return false;
		}
		return mouse_states[idx].pressed;
	}

	bool InputFrame::MouseButtonReleased(MouseButton button) const
	{
		int idx = static_cast<int>(button);
		if (idx < 0 || idx >= static_cast<int>(MouseButton::Count)) 
		{
			return false;
		}
		return mouse_states[idx].released;
	}

	InputVector2 InputFrame::GetMousePosition() const
	{
		return mouse_position;
	}

	InputVector2 InputFrame::GetMouseDelta() const
	{
		return mouse_delta;
	}

	InputVector2 InputFrame::GetScrollDelta() const
	{
		return scroll_delta;
	}

	bool InputFrame::IsActionDown(InputActionId action) const
	{
		uint32_t idx = action.value;
		if (idx >= action_states.size()) 
		{
			return false;
		}
		return action_states[idx].down;
	}

	bool InputFrame::WasActionPressed(InputActionId action) const
	{
		uint32_t idx = action.value;
		if (idx >= action_states.size()) 
		{
			return false;
		}
		return action_states[idx].pressed;
	}

	bool InputFrame::WasActionReleased(InputActionId action) const
	{
		uint32_t idx = action.value;
		if (idx >= action_states.size()) 
		{
			return false;
		}
		return action_states[idx].released;
	}

	float InputFrame::GetAxis(InputActionId action) const
	{
		uint32_t idx = action.value;
		if (idx >= action_states.size()) 
		{
			return 0.0f;
		}
		return action_states[idx].scalar;
	}

	InputVector2 InputFrame::GetVector2(InputActionId action) const
	{
		uint32_t idx = action.value;
		if (idx >= action_states.size()) 
		{
			return {};
		}
		return action_states[idx].vector2;
	}
}