#pragma once

#include <engine/engine_api.h>
#include <engine/input/input_types.h>
#include <engine/input/input_action_map.h>

#include <string>
#include <string_view>
#include <vector>

namespace Engine
{
	enum class InputConsumptionPolicy
	{
		Consume,
		PassThrough
	};

	struct InputContextDef
	{
		InputContextId id;
		std::string name;
		int priority = 0;
		InputConsumptionPolicy consumption = InputConsumptionPolicy::Consume;
	};

	class ENGINE_API InputContextStack
	{
	public:
		InputContextId RegisterContext(std::string_view name, int priority, InputConsumptionPolicy policy);

		void PushContext(InputContextId id);
		void PopContext();
		void RemoveContext(InputContextId id);
		void EnableContext(InputContextId id);
		void DisableContext(InputContextId id);

		bool IsContextActive(InputContextId id) const;
		std::vector<InputContextId> GetActiveContexts() const;

		//const std::vector<InputContextDef>& GetContexts() const;

	private:
		struct RuntimeContext
		{
			InputContextDef def;
			bool active = false;
			bool enabled = true;
		};

		std::vector<RuntimeContext> m_contexts;
		std::vector<InputContextId> m_stack;
		uint32_t m_next_context_id = 0;
	};
}