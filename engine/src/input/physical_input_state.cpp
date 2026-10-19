#include "physical_input_state.h"

namespace Engine
{
	PhysicalInputState::PhysicalInputState() = default;

	void PhysicalInputState::ApplyEvent(const InputEvent& event)
	{
		switch (event.type)
		{
		case InputEventType::KeyDown:
		{
			if (auto* e = event.AsKey())
			{
				SetKeyState(e->key, true);
			}
			break;
		}
		case InputEventType::KeyUp:
		{
			if (auto* e = event.AsKey())
			{
				SetKeyState(e->key, false);
			}
			break;
		}
		case InputEventType::KeyRepeat:
			break;
		case InputEventType::MouseButtonDown:
		{
			if (auto* e = event.AsMouseButton())
			{
				SetMouseButtonState(e->button, true);
			}
			break;
		}
		case InputEventType::MouseButtonUp:
		{
			if (auto* e = event.AsMouseButton())
			{
				SetMouseButtonState(e->button, false);
			}
			break;
		}
		case InputEventType::MouseMove:
		{
			if (auto* e = event.AsMouseMove())
			{
				const InputVector2 position{ static_cast<float>(e->xpos), static_cast<float>(e->ypos) };
				if (m_mouse_position_valid)
				{
					m_mouse_delta.x += static_cast<float>(e->xpos) - m_mouse_position.x;
					m_mouse_delta.y += static_cast<float>(e->ypos) - m_mouse_position.y;
				}
				m_mouse_position = position;
				m_mouse_position_valid = true;
			}
			break;
		}
		case InputEventType::MouseScroll:
		{
			if (auto* e = event.AsMouseScroll())
			{
				m_scroll_delta.x += static_cast<float>(e->xoffset);
				m_scroll_delta.y += static_cast<float>(e->yoffset);
			}
			break;
		}
		case InputEventType::WindowFocusGained:
			m_focused = true;
			m_mouse_position_valid = false;
			break;
		case InputEventType::WindowFocusLost:
			m_focused = false;
			m_mouse_position_valid = false;
			ReleaseAll();
			break;
		default:
			break;
		}
	}

	void PhysicalInputState::BeginFrame()
	{
		for (auto& k : m_keys)
		{
			k.pressed = false;
			k.released = false;
		}
		for (auto& m : m_mouse_buttons)
		{
			m.pressed = false;
			m.released = false;
		}
		m_mouse_delta = {};
		m_scroll_delta = {};
	}

	void PhysicalInputState::EndFrame()
	{
	}

	bool PhysicalInputState::IsKeyDown(Key key) const
	{
		int idx = static_cast<int>(key);
		if (idx < 0 || idx >= static_cast<int>(Key::Count)) return false;
		return m_keys[idx].down;
	}

	bool PhysicalInputState::IsKeyPressed(Key key) const
	{
		int idx = static_cast<int>(key);
		if (idx < 0 || idx >= static_cast<int>(Key::Count)) return false;
		return m_keys[idx].pressed;
	}

	bool PhysicalInputState::IsKeyReleased(Key key) const
	{
		int idx = static_cast<int>(key);
		if (idx < 0 || idx >= static_cast<int>(Key::Count)) return false;
		return m_keys[idx].released;
	}

	bool PhysicalInputState::IsMouseButtonDown(MouseButton button) const
	{
		int idx = static_cast<int>(button);
		if (idx < 0 || idx >= static_cast<int>(MouseButton::Count)) return false;
		return m_mouse_buttons[idx].down;
	}

	bool PhysicalInputState::IsMouseButtonPressed(MouseButton button) const
	{
		int idx = static_cast<int>(button);
		if (idx < 0 || idx >= static_cast<int>(MouseButton::Count)) return false;
		return m_mouse_buttons[idx].pressed;
	}

	bool PhysicalInputState::IsMouseButtonReleased(MouseButton button) const
	{
		int idx = static_cast<int>(button);
		if (idx < 0 || idx >= static_cast<int>(MouseButton::Count)) return false;
		return m_mouse_buttons[idx].released;
	}

	InputVector2 PhysicalInputState::GetMousePosition() const
	{
		return m_mouse_position;
	}

	InputVector2 PhysicalInputState::GetMouseDelta() const
	{
		return m_mouse_delta;
	}

	InputVector2 PhysicalInputState::GetScrollDelta() const
	{
		return m_scroll_delta;
	}

	void PhysicalInputState::ReleaseAll()
	{
		for (auto& k : m_keys)
		{
			if (k.down)
			{
				k.down = false;
				k.released = true;
			}
		}
		for (auto& m : m_mouse_buttons)
		{
			if (m.down)
			{
				m.down = false;
				m.released = true;
			}
		}
		m_mouse_delta = {};
		m_scroll_delta = {};
		UpdateModifiers();
	}

	void PhysicalInputState::UpdateModifiers()
	{
		m_modifiers = ModifierKey::None;
		if (IsKeyDown(Key::LeftShift) || IsKeyDown(Key::RightShift))
			m_modifiers = m_modifiers | ModifierKey::Shift;
		if (IsKeyDown(Key::LeftControl) || IsKeyDown(Key::RightControl))
			m_modifiers = m_modifiers | ModifierKey::Control;
		if (IsKeyDown(Key::LeftAlt) || IsKeyDown(Key::RightAlt))
			m_modifiers = m_modifiers | ModifierKey::Alt;
		if (IsKeyDown(Key::LeftSuper) || IsKeyDown(Key::RightSuper))
			m_modifiers = m_modifiers | ModifierKey::Super;
	}

	void PhysicalInputState::SetKeyState(Key key, bool down)
	{
		int idx = static_cast<int>(key);
		if (idx < 0 || idx >= static_cast<int>(Key::Count)) return;

		auto& state = m_keys[idx];
		if (down && !state.down)
		{
			state.pressed = true;
		}
		else if (!down && state.down)
		{
			state.released = true;
		}
		state.down = down;
		UpdateModifiers();
	}

	void PhysicalInputState::SetMouseButtonState(MouseButton button, bool down)
	{
		int idx = static_cast<int>(button);
		if (idx < 0 || idx >= static_cast<int>(MouseButton::Count)) return;

		auto& state = m_mouse_buttons[idx];
		if (down && !state.down)
		{
			state.pressed = true;
		}
		else if (!down && state.down)
		{
			state.released = true;
		}
		state.down = down;
	}
}