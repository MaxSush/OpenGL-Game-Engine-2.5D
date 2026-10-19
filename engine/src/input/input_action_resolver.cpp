#include "input_action_resolver.h"
#include "physical_input_state.h"

#include <algorithm>
#include <cmath>

namespace Engine
{
	InputActionResolver::InputActionResolver() = default;

	void InputActionResolver::SetActionMap(const InputActionMap* map)
	{
		m_action_map = map;
		m_actions.clear();
		m_previous_actions.clear();

		if (m_action_map)
		{
			for (const auto& def : m_action_map->GetActions())
			{
				ResolvedAction ra;
				ra.id = def.id;
				ra.type = def.type;
				m_actions.push_back(ra);
				m_previous_actions.push_back(ra);
			}
		}
	}

	void InputActionResolver::Resolve(const PhysicalInputState& state, const std::vector<InputContextId>& active_contexts)
	{
		m_previous_actions = m_actions;

		for (auto& a : m_actions)
		{
			a.pressed = false;
			a.released = false;
			a.down = false;
			a.scalar = 0.0f;
			a.vector2 = {};
		}

		if (!m_action_map) return;

		for (const auto& binding : m_action_map->GetBindings())
		{
			EvaluateBinding(binding, state);
		}

		for (auto& a : m_actions)
		{
			if (a.type == InputActionType::Vector2)
			{
				float mag = std::sqrt(a.vector2.x * a.vector2.x + a.vector2.y * a.vector2.y);
				if (mag > 1.0f)
				{
					a.vector2.x /= mag;
					a.vector2.y /= mag;
				}
				a.down = mag > 0.0001f;
			}
			else if (a.type == InputActionType::Scalar)
			{
				a.down = std::abs(a.scalar) > 0.0001f;
			}
		}

		ComputeTransitions();
	}

	void InputActionResolver::EvaluateBinding(const InputBinding& binding, const PhysicalInputState& state)
	{
		auto it = std::find_if
		(
			m_actions.begin(), 
			m_actions.end(),
			[&](const ResolvedAction& a) 
			{ 
				return a.id == binding.action; 
			}
		);
		if (it == m_actions.end()) return;

		if (binding.required_mods != ModifierKey::None)
		{
			if (!HasModifier(state.GetModifiers(), binding.required_mods))
			{
				return;
			}
		}

		float contribution = 0.0f;
		bool active = false;

		switch (binding.source)
		{
		case InputBindingSource::Key:
			active = state.IsKeyDown(binding.key);
			contribution = (active) ? binding.scale : 0.0f;
			if (state.IsKeyPressed(binding.key))
			{
				it->pressed = true;
			}
			if (state.IsKeyReleased(binding.key))
			{
				it->released = true;
			}
			break;
		case InputBindingSource::MouseButton:
			active = state.IsMouseButtonDown(binding.mouse_button);
			contribution = (active) ? binding.scale : 0.0f;
			if (state.IsMouseButtonPressed(binding.mouse_button))
			{
				it->pressed = true;
			}
			if(state.IsMouseButtonReleased(binding.mouse_button))
			{
				it->released = true;
			}
			break;
		case InputBindingSource::MouseAxisX:
			contribution = state.GetMouseDelta().x * binding.scale;
			active = std::abs(contribution) > 0.0001f;
			break;
		case InputBindingSource::MouseAxisY:
			contribution = state.GetMouseDelta().y * binding.scale;
			active = std::abs(contribution) > 0.0001f;
			break;
		case InputBindingSource::MouseScrollX:
			contribution = state.GetScrollDelta().x * binding.scale;
			active = std::abs(contribution) > 0.0001f;
			break;
		case InputBindingSource::MouseScrollY:
			contribution = state.GetScrollDelta().y * binding.scale;
			active = std::abs(contribution) > 0.0001f;
			break;
		default:
			break;
		}

		if (!active) return;

		switch (it->type)
		{
		case InputActionType::Digital:
			it->down = true;
			break;
		case InputActionType::Scalar:
			it->scalar += contribution;
			it->scalar = std::clamp(it->scalar, -1.0f, 1.0f);
			break;
		case InputActionType::Vector2:
			if (binding.component == InputVectorComponent::X)
				it->vector2.x += contribution;
			else
				it->vector2.y += contribution;
			break;
		}
	}

	void InputActionResolver::ComputeTransitions()
	{
		for (auto& cur : m_actions)
		{
			auto prev = std::find_if
			(
				m_previous_actions.begin(), 
				m_previous_actions.end(),
				[&](const ResolvedAction& a) 
				{ 
					return a.id == cur.id; 
				}
			);

			bool prev_down = (prev != m_previous_actions.end() && prev->down);
			cur.pressed = cur.pressed || (cur.down && !prev_down);
			cur.released = cur.released || (!cur.down && prev_down);
		}
	}

	const ResolvedAction* InputActionResolver::FindAction(InputActionId id) const
	{
		auto it = std::find_if(m_actions.begin(), m_actions.end(),
			[&](const ResolvedAction& a) { return a.id == id; });
		return it != m_actions.end() ? &(*it) : nullptr;
	}
}