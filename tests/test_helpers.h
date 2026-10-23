#pragma once

#include <engine/input/input_types.h>

namespace TestUtil
{
	// Key
	inline Engine::InputEvent MakeKey(Engine::InputEventType type, Engine::Key key)
	{
		Engine::InputEvent e;
		e.type = type;
		e.payload = Engine::KeyEvent{ key, Engine::ModifierKey::None };
		return e;
	}

	inline Engine::InputEvent KeyDown(Engine::Key k) 
	{ 
		return MakeKey(Engine::InputEventType::KeyDown, k); 
	}

	inline Engine::InputEvent KeyUp(Engine::Key k) 
	{ 
		return MakeKey(Engine::InputEventType::KeyUp, k); 
	}

	// Mouse
	inline Engine::InputEvent MakeMouseButton(Engine::InputEventType type, Engine::MouseButton b)
	{
		Engine::InputEvent e;
		e.type = type;
		e.payload = Engine::MouseButtonEvent{ b, Engine::ModifierKey::None };
		return e;
	}

	inline Engine::InputEvent MouseDown(Engine::MouseButton b) 
	{ 
		return MakeMouseButton(Engine::InputEventType::MouseButtonDown, b); 
	}

	inline Engine::InputEvent MouseUp(Engine::MouseButton b) 
	{ 
		return MakeMouseButton(Engine::InputEventType::MouseButtonUp, b); 
	}

	inline Engine::InputEvent MouseMove(double x, double y)
	{
		Engine::InputEvent e;
		e.type = Engine::InputEventType::MouseMove;
		e.payload = Engine::MouseMoveEvent{ x, y };
		return e;
	}

	inline Engine::InputEvent Scroll(double x, double y)
	{
		Engine::InputEvent e;
		e.type = Engine::InputEventType::MouseScroll;
		e.payload = Engine::MouseScrollEvent{ x, y };
		return e;
	}

	inline Engine::InputEvent Focus(bool gained)
	{
		Engine::InputEvent e;
		e.type = gained ? Engine::InputEventType::WindowFocusGained : Engine::InputEventType::WindowFocusLost;
		return e;
	}

}