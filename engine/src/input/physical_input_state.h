#pragma once

#include <engine/input/input_types.h>

#include <array>

namespace Engine
{
	class PhysicalInputState
	{
	public:
		PhysicalInputState();

		void ApplyEvent(const InputEvent& event);

		void BeginFrame();
		void EndFrame();

		bool IsKeyDown(Key key) const;
		bool IsKeyPressed(Key key) const;
		bool IsKeyReleased(Key key) const;

		bool IsMouseButtonDown(MouseButton button) const;
		bool IsMouseButtonPressed(MouseButton button) const;
		bool IsMouseButtonReleased(MouseButton button) const;

		InputVector2 GetMousePosition() const;
		InputVector2 GetMouseDelta() const;
		InputVector2 GetScrollDelta() const;

		ModifierKey GetModifiers() const { return m_modifiers; }
		bool HasFocus() const { return m_focused; }

		void ReleaseAll();

	private:
		struct KeyState
		{
			bool down = false;
			bool pressed = false;
			bool released = false;
		};

		struct MouseState
		{
			bool down = false;
			bool pressed = false;
			bool released = false;
		};

		std::array<KeyState, static_cast<int>(Key::Count)> m_keys{};
		std::array<MouseState, static_cast<int>(MouseButton::Count)> m_mouse_buttons{};

		InputVector2 m_mouse_position{};
		InputVector2 m_mouse_delta{};
		InputVector2 m_scroll_delta{};
		bool m_mouse_position_valid = false;

		ModifierKey m_modifiers = ModifierKey::None;
		bool m_focused = true;

		void UpdateModifiers();
		void SetKeyState(Key key, bool down);
		void SetMouseButtonState(MouseButton button, bool down);
	};
}