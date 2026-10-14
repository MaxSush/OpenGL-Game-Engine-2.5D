#include <engine/input/input.h>
#include <engine/platform/platform_window.h>

#include "input_system.h"

namespace Engine
{
	Input::Input(Window& window)
		: m_system(std::make_unique<InputSystem>())
	{
		m_system->SetBackend(window.CreateInputBackend());
	}

	Input::~Input() = default;

	const InputFrame& Input::GetFrame() const
	{
		return m_system->GetFrame();
	}

	bool Input::FinalizeActionSetup()
	{
		return m_system->FinalizeActionSetup();
	}

	bool Input::IsKeyDown(Key key) const
	{
		return m_system->GetFrame().KeyDown(key);
	}

	bool Input::WasKeyPressed(Key key) const
	{
		return m_system->GetFrame().KeyPressed(key);
	}

	bool Input::WasKeyReleased(Key key) const
	{
		return m_system->GetFrame().KeyReleased(key);
	}

	bool Input::IsMouseButtonDown(MouseButton button) const
	{
		return m_system->GetFrame().MouseButtonDown(button);
	}

	bool Input::WasMouseButtonPressed(MouseButton button) const
	{
		return m_system->GetFrame().MouseButtonPressed(button);
	}

	bool Input::WasMouseButtonReleased(MouseButton button) const
	{
		return m_system->GetFrame().MouseButtonReleased(button);
	}

	InputVector2 Input::GetMousePosition() const
	{
		return m_system->GetFrame().GetMousePosition();
	}

	InputVector2 Input::GetMouseDelta() const
	{
		return m_system->GetFrame().GetMouseDelta();
	}

	InputVector2 Input::GetScrollDelta() const
	{
		return m_system->GetFrame().GetScrollDelta();
	}

	bool Input::IsActionDown(InputActionId action) const
	{
		return m_system->GetFrame().IsActionDown(action);
	}

	bool Input::WasActionPressed(InputActionId action) const
	{
		return m_system->GetFrame().WasActionPressed(action);
	}

	bool Input::WasActionReleased(InputActionId action) const
	{
		return m_system->GetFrame().WasActionReleased(action);
	}

	float Input::GetAxis(InputActionId action) const
	{
		return m_system->GetFrame().GetAxis(action);
	}

	InputVector2 Input::GetVector2(InputActionId action) const
	{
		return m_system->GetFrame().GetVector2(action);
	}

	InputActionMap& Input::GetActionMap()
	{
		return m_system->GetActionMap();
	}

	const InputActionMap& Input::GetActionMap() const
	{
		return m_system->GetActionMap();
	}

	InputContextStack& Input::GetContexts()
	{
		return m_system->GetContexts();
	}

	const InputContextStack& Input::GetContexts() const
	{
		return m_system->GetContexts();
	}

	void Input::SetCursorMode(CursorMode mode)
	{
		m_system->SetCursorMode(mode);
	}

	CursorMode Input::GetCursorMode() const
	{
		return m_system->GetCursorMode();
	}

	void Input::BeginFrame()
	{
		m_system->BeginFrame();
	}

	void Input::EndFrame()
	{
		m_system->EndFrame();
	}
}