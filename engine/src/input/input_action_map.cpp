#include <engine/input/input_action_map.h>

#include <algorithm>

namespace Engine
{
	InputActionId InputActionMap::RegisterAction(std::string_view name, InputActionType type)
	{
		auto it = std::find_if
		(
			m_actions.begin(), 
			m_actions.end(),
			[&](const InputActionDef& a) 
			{ 
				return a.name == name; 
			}
		);
		if (it != m_actions.end())
		{
			return it->id;
		}

		if (m_actions_finalized)
		{
			return {};
		}

		InputActionDef def;
		def.id = { m_next_action_id++ };
		def.name = std::string(name);
		def.type = type;
		m_actions.push_back(def);
		return def.id;
	}

	InputActionId InputActionMap::FindAction(std::string_view name) const
	{
		auto it = std::find_if(m_actions.begin(), m_actions.end(),
			[&](const InputActionDef& a) { return a.name == name; });
		if (it != m_actions.end())
		{
			return it->id;
		}
		return {};
	}

	void InputActionMap::AddBinding(const InputBinding& binding)
	{
		m_bindings.push_back(binding);
	}

	void InputActionMap::FinalizeActions() 
	{
		m_actions_finalized = true;
	}

	InputActionType InputActionMap::GetActionType(InputActionId id) const
	{
		auto it = std::find_if(m_actions.begin(), m_actions.end(),
			[&](const InputActionDef& a) { return a.id == id; });
		if (it != m_actions.end())
		{
			return it->type;
		}
		return InputActionType::Digital;
	}
}