#include <engine/input/input_context.h>

#include <algorithm>

namespace Engine
{
	InputContextId InputContextStack::RegisterContext(std::string_view name, int priority, InputConsumptionPolicy policy)
	{
		InputContextDef def;
		def.id = { m_next_context_id++ };
		def.name = std::string(name);
		def.priority = priority;
		def.consumption = policy;

		RuntimeContext ctx;
		ctx.def = def;
		ctx.active = false;
		ctx.enabled = true;
		m_contexts.push_back(ctx);
		return def.id;
	}

	void InputContextStack::PushContext(InputContextId id)
	{
		auto it = std::find_if(m_contexts.begin(), m_contexts.end(),
			[&](const RuntimeContext& c) { return c.def.id == id; });
		if (it == m_contexts.end()) return;

		if (std::find(m_stack.begin(), m_stack.end(), id) == m_stack.end())
		{
			it->active = true;
			m_stack.push_back(id);
		}
	}

	void InputContextStack::PopContext()
	{
		if (m_stack.empty()) return;
		InputContextId id = m_stack.back();
		m_stack.pop_back();

		auto it = std::find_if(m_contexts.begin(), m_contexts.end(),
			[&](const RuntimeContext& c) { return c.def.id == id; });
		if (it != m_contexts.end())
		{
			it->active = false;
		}
	}

	void InputContextStack::RemoveContext(InputContextId id)
	{
		m_stack.erase(std::remove(m_stack.begin(), m_stack.end(), id), m_stack.end());
		m_contexts.erase(std::remove_if(m_contexts.begin(), m_contexts.end(),
			[&](const RuntimeContext& c) { return c.def.id == id; }), m_contexts.end());
	}

	void InputContextStack::EnableContext(InputContextId id)
	{
		auto it = std::find_if(m_contexts.begin(), m_contexts.end(),
			[&](const RuntimeContext& c) { return c.def.id == id; });
		if (it != m_contexts.end())
		{
			it->enabled = true;
		}
	}

	void InputContextStack::DisableContext(InputContextId id)
	{
		auto it = std::find_if(m_contexts.begin(), m_contexts.end(),
			[&](const RuntimeContext& c) { return c.def.id == id; });
		if (it != m_contexts.end())
		{
			it->enabled = false;
			it->active = false;
		}
		m_stack.erase(std::remove(m_stack.begin(), m_stack.end(), id), m_stack.end());
	}

	bool InputContextStack::IsContextActive(InputContextId id) const
	{
		auto it = std::find_if(m_contexts.begin(), m_contexts.end(),
			[&](const RuntimeContext& c) { return c.def.id == id; });
		if (it == m_contexts.end()) return false;
		return it->active && it->enabled;
	}

	std::vector<InputContextId> InputContextStack::GetActiveContexts() const
	{
		std::vector<RuntimeContext> active;
		for (const auto& ctx : m_contexts)
		{
			if (ctx.active && ctx.enabled)
			{
				active.push_back(ctx);
			}
		}

		std::sort(active.begin(), active.end(),
			[](const RuntimeContext& a, const RuntimeContext& b)
			{
				return a.def.priority > b.def.priority;
			});

		std::vector<InputContextId> result;
		result.reserve(active.size());
		for (const auto& ctx : active)
		{
			result.push_back(ctx.def.id);
		}
		return result;
	}

	//const std::vector<InputContextDef>& InputContextStack::GetContexts() const
	//{
	//	return m_contexts;
	//}
}