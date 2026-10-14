#pragma once

#include <engine/engine_api.h>
#include <engine/input/input_types.h>

#include <string>
#include <string_view>
#include <vector>

namespace Engine
{
	enum class InputBindingSource
	{
		None,
		Key,
		MouseButton,
		MouseAxisX,
		MouseAxisY,
		MouseScrollX,
		MouseScrollY
	};

	enum class InputVectorComponent
	{
		X,
		Y
	};

	struct InputBinding
	{
		InputActionId action;
		InputBindingSource source = InputBindingSource::None;
		Key key = Key::Unknown;
		MouseButton mouse_button = MouseButton::None;
		ModifierKey required_mods = ModifierKey::None;
		float scale = 1.0f;
		InputVectorComponent component = InputVectorComponent::X;
	};

	struct InputActionDef
	{
		InputActionId id;
		std::string name;
		InputActionType type = InputActionType::Digital;
	};

	class ENGINE_API InputActionMap
	{
	public:
		InputActionId RegisterAction(std::string_view name, InputActionType type);
		InputActionId FindAction(std::string_view name) const;

		void AddBinding(const InputBinding& binding);
		void FinalizeActions();

		bool AreActionsFinalized() const { return m_actions_finalized; }

		const std::vector<InputActionDef>& GetActions() const { return m_actions; }
		const std::vector<InputBinding>& GetBindings() const { return m_bindings; }

		InputActionType GetActionType(InputActionId id) const;

	private:
		std::vector<InputActionDef> m_actions;
		std::vector<InputBinding> m_bindings;
		uint32_t m_next_action_id = 0;
		bool m_actions_finalized = false;
	};
}