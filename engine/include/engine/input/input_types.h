#pragma once

#include <engine/engine_api.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

namespace Engine
{
	enum class Key : int
	{
		Unknown = -1,
		Space = 32,
		Apostrophe = 39,
		Comma = 44,
		Minus = 45,
		Period = 46,
		Slash = 47,
		Num0 = 48, Num1 = 49, Num2 = 50, Num3 = 51, Num4 = 52,
		Num5 = 53, Num6 = 54, Num7 = 55, Num8 = 56, Num9 = 57,
		Semicolon = 59,
		Equal = 61,
		A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71,
		H = 72, I = 73, J = 74, K = 75, L = 76, M = 77, N = 78,
		O = 79, P = 80, Q = 81, R = 82, S = 83, T = 84, U = 85,
		V = 86, W = 87, X = 88, Y = 89, Z = 90,
		LeftBracket = 91,
		Backslash = 92,
		RightBracket = 93,
		GraveAccent = 96,
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,	
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290, F2 = 291, F3 = 292, F4 = 293, F5 = 294, F6 = 295,
		F7 = 296, F8 = 297, F9 = 298, F10 = 299, F11 = 300, F12 = 301,
		Keypad0 = 320, Keypad1 = 321, Keypad2 = 322, Keypad3 = 323,
		Keypad4 = 324, Keypad5 = 325, Keypad6 = 326, Keypad7 = 327,
		Keypad8 = 328, Keypad9 = 329,
		KeypadDecimal = 330,
		KeypadDivide = 331,
		KeypadMultiply = 332,
		KeypadSubtract = 333,
		KeypadAdd = 334,
		KeypadEnter = 335,
		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348,
		Count = 349
	};

	enum class MouseButton : int
	{
		None = -1,
		Left = 0,
		Right = 1,
		Middle = 2,
		Button4 = 3,
		Button5 = 4,
		Button6 = 5,
		Button7 = 6,
		Button8 = 7,
		Count = 8
	};

	enum class ModifierKey : int
	{
		None = 0,
		Shift = 0x0001,
		Control = 0x0002,
		Alt = 0x0004,
		Super = 0x0008,
		CapsLock = 0x0010,
		NumLock = 0x0020
	};

	inline ModifierKey operator|(ModifierKey a, ModifierKey b)
	{
		return static_cast<ModifierKey>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline ModifierKey operator&(ModifierKey a, ModifierKey b)
	{
		return static_cast<ModifierKey>(static_cast<int>(a) & static_cast<int>(b));
	}

	inline bool HasModifier(ModifierKey state, ModifierKey flag)
	{
		return (state & flag) == flag;
	}

	enum class CursorMode
	{
		Visible,
		Hidden,
		Captured
	};

	enum class InputActionType
	{
		Digital,
		Scalar,
		Vector2
	};

	struct InputActionId
	{
		uint32_t value = InvalidId;
		static constexpr uint32_t InvalidId = 0xFFFFFFFFu;
		bool IsValid() const { return value != InvalidId; }
		bool operator==(const InputActionId&) const = default;
	};

	struct InputContextId
	{
		uint32_t value = InvalidId;
		static constexpr uint32_t InvalidId = 0xFFFFFFFFu;
		bool IsValid() const { return value != InvalidId; }
		bool operator==(const InputContextId&) const = default;
	};

	struct InputVector2
	{
		float x = 0.0f;
		float y = 0.0f;
	};

	struct InputValue
	{
		InputActionType type = InputActionType::Digital;
		bool digital = false;
		float scalar = 0.0f;
		InputVector2 vector2{};

		bool IsDigital() const { return type == InputActionType::Digital; }
		bool IsScalar() const { return type == InputActionType::Scalar; }
		bool IsVector2() const { return type == InputActionType::Vector2; }
	};

	enum class InputEventType
	{
		None,
		KeyDown,
		KeyUp,
		KeyRepeat,
		MouseButtonDown,
		MouseButtonUp,
		MouseMove,
		MouseScroll,
		WindowFocusGained,
		WindowFocusLost,
		CursorEnter,
		CursorLeave,
		TextInput
	};

	struct KeyEvent
	{
		Key key = Key::Unknown;
		ModifierKey mods = ModifierKey::None;
	};

	struct MouseButtonEvent
	{
		MouseButton button = MouseButton::None;
		ModifierKey mods = ModifierKey::None;
	};

	struct MouseMoveEvent
	{
		double xpos = 0.0;
		double ypos = 0.0;
	};

	struct MouseScrollEvent
	{
		double xoffset = 0.0;
		double yoffset = 0.0;
	};

	struct TextInputEvent
	{
		uint32_t codepoint = 0;
	};

	struct InputEvent
	{
		using Payload = std::variant<
			std::monostate,
			KeyEvent,
			MouseButtonEvent,
			MouseMoveEvent,
			MouseScrollEvent,
			TextInputEvent
		>;

		InputEventType type = InputEventType::None;
		uint64_t sequence = 0;
		Payload payload;

		const KeyEvent* AsKey() const { return std::get_if<KeyEvent>(&payload); }
		const MouseButtonEvent* AsMouseButton() const { return std::get_if<MouseButtonEvent>(&payload); }
		const MouseMoveEvent* AsMouseMove() const { return std::get_if<MouseMoveEvent>(&payload); }
		const MouseScrollEvent* AsMouseScroll() const { return std::get_if<MouseScrollEvent>(&payload); }
		const TextInputEvent* AsTextInput() const { return std::get_if<TextInputEvent>(&payload); }
	};
}