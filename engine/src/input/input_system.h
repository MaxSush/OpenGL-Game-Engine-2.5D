#pragma once

#include <engine/input/input_types.h>
#include <engine/input/input_frame.h>
#include <engine/input/input_action_map.h>
#include <engine/input/input_context.h>
#include <engine/platform/input_backend.h>

#include "input_event_queue.h"
#include "physical_input_state.h"
#include "input_action_resolver.h"

#include <memory>

namespace Engine
{
	class InputSystem
	{
	public:
		InputSystem();
		~InputSystem();

		void SetBackend(std::unique_ptr<InputBackend> backend);

		void BeginFrame();
		void EndFrame();

		bool FinalizeActionSetup();

		const InputFrame& GetFrame() const { return m_frame; }

		InputActionMap& GetActionMap() { return m_action_map; }
		const InputActionMap& GetActionMap() const { return m_action_map; }

		InputContextStack& GetContexts() { return m_contexts; }
		const InputContextStack& GetContexts() const { return m_contexts; }

		void SetCursorMode(CursorMode mode);
		CursorMode GetCursorMode() const;

	private:
		void ProcessEvents();
		void PublishFrame();

		std::unique_ptr<InputBackend> m_backend;
		InputEventQueue m_event_queue;
		PhysicalInputState m_physical_state;
		InputActionResolver m_resolver;
		InputActionMap m_action_map;
		InputContextStack m_contexts;
		InputFrame m_frame;
		uint64_t m_frame_id = 0;
	};
}