#include "input_system.h"
#include <engine/core/logger/log.h>

namespace Engine
{
	InputSystem::InputSystem()
	{
	}

	InputSystem::~InputSystem() = default;

	void InputSystem::SetBackend(std::unique_ptr<InputBackend> backend)
	{
		m_backend = std::move(backend);
		if (m_backend)
		{
			m_backend->SetEventQueue(&m_event_queue);
		}
	}

	void InputSystem::BeginFrame()
	{
		m_physical_state.BeginFrame();
		ProcessEvents();

		if (m_action_map.AreActionsFinalized())
		{
			m_resolver.Resolve(m_physical_state, m_contexts.GetActiveContexts());
		}

		PublishFrame();
	}

	void InputSystem::EndFrame()
	{
		m_physical_state.EndFrame();
	}

	bool InputSystem::FinalizeActionSetup()
	{
		if (m_action_map.AreActionsFinalized())
		{
			LOG_ERROR("InputSystem::FinalizeActionSetup => Actions Finalized already");
			return false;
		}

		m_action_map.FinalizeActions();
		m_resolver.SetActionMap(&m_action_map);

		return true;
	}

	void InputSystem::SetCursorMode(CursorMode mode)
	{
		if (m_backend)
		{
			m_backend->SetCursorMode(mode);
		}
	}

	CursorMode InputSystem::GetCursorMode() const
	{
		if (m_backend)
		{
			return m_backend->GetCursorMode();
		}
		return CursorMode::Visible;
	}

	void InputSystem::ProcessEvents()
	{
		InputEvent event;
		while (m_event_queue.Pop(event))
		{
			m_physical_state.ApplyEvent(event);
		}

		if (m_event_queue.Overflowed())
		{
			LOG_WARN("InputSystem::ProcessEvents => event queue overflowed, input events were dropped");
			m_event_queue.Clear();
		}
	}

	void InputSystem::PublishFrame()
	{
		m_frame.frame_id = ++m_frame_id;

		for (int i = 0; i < static_cast<int>(Key::Count); ++i)
		{
			Key key = static_cast<Key>(i);
			m_frame.key_states[i].down = m_physical_state.IsKeyDown(key);
			m_frame.key_states[i].pressed = m_physical_state.IsKeyPressed(key);
			m_frame.key_states[i].released = m_physical_state.IsKeyReleased(key);
		}

		for (int i = 0; i < static_cast<int>(MouseButton::Count); ++i)
		{
			MouseButton btn = static_cast<MouseButton>(i);
			m_frame.mouse_states[i].down = m_physical_state.IsMouseButtonDown(btn);
			m_frame.mouse_states[i].pressed = m_physical_state.IsMouseButtonPressed(btn);
			m_frame.mouse_states[i].released = m_physical_state.IsMouseButtonReleased(btn);
		}

		m_frame.mouse_position = m_physical_state.GetMousePosition();
		m_frame.mouse_delta = m_physical_state.GetMouseDelta();
		m_frame.scroll_delta = m_physical_state.GetScrollDelta();

		m_frame.action_states.clear();
		for (const auto& action : m_resolver.GetActions())
		{
			InputFrame::ActionState state;
			state.down = action.down;
			state.pressed = action.pressed;
			state.released = action.released;
			state.scalar = action.scalar;
			state.vector2 = action.vector2;
			m_frame.action_states.push_back(state);
		}
	}
}