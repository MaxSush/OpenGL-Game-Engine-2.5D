#pragma once

#include <engine/engine_api.h>
#include <engine/input/input_types.h>
#include <engine/input/input_frame.h>
#include <engine/input/input_action_map.h>
#include <engine/input/input_context.h>

#include <memory>

namespace Engine
{
	class Window;
	class InputSystem;

	class ENGINE_API Input
	{
	public:
		explicit Input(Window& window);
		~Input();

		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;
		Input(Input&&) = delete;
		Input& operator=(Input&&) = delete;

		const InputFrame& GetFrame() const;
		bool FinalizeActionSetup();

		bool IsKeyDown(Key key) const;
		bool WasKeyPressed(Key key) const;
		bool WasKeyReleased(Key key) const;

		bool IsMouseButtonDown(MouseButton button) const;
		bool WasMouseButtonPressed(MouseButton button) const;
		bool WasMouseButtonReleased(MouseButton button) const;

		InputVector2 GetMousePosition() const;
		InputVector2 GetMouseDelta() const;
		InputVector2 GetScrollDelta() const;

		bool IsActionDown(InputActionId action) const;
		bool WasActionPressed(InputActionId action) const;
		bool WasActionReleased(InputActionId action) const;
		float GetAxis(InputActionId action) const;
		InputVector2 GetVector2(InputActionId action) const;

		InputActionMap& GetActionMap();
		const InputActionMap& GetActionMap() const;

		InputContextStack& GetContexts();
		const InputContextStack& GetContexts() const;

		void SetCursorMode(CursorMode mode);
		CursorMode GetCursorMode() const;

		void BeginFrame();
		void EndFrame();

	private:
		std::unique_ptr<InputSystem> m_system;
	};
}