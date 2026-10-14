#pragma once

#include <engine/input/input_types.h>
#include <engine/input/input_action_map.h>

#include <vector>

namespace Engine
{
	class PhysicalInputState;
	struct InputFrame;

	struct ResolvedAction
	{
		InputActionId id;
		InputActionType type = InputActionType::Digital;
		bool down = false;
		bool pressed = false;
		bool released = false;
		float scalar = 0.0f;
		InputVector2 vector2{};
	};

	class InputActionResolver
	{
	public:
		InputActionResolver();

		void SetActionMap(const InputActionMap* map);
		void Resolve(const PhysicalInputState& state, const std::vector<InputContextId>& active_contexts);

		const std::vector<ResolvedAction>& GetActions() const { return m_actions; }
		const ResolvedAction* FindAction(InputActionId id) const;

	private:
		void EvaluateBinding(const InputBinding& binding, const PhysicalInputState& state);
		void ComputeTransitions();

		const InputActionMap* m_action_map = nullptr;
		std::vector<ResolvedAction> m_actions;
		std::vector<ResolvedAction> m_previous_actions;
	};
}