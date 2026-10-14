#pragma once

#include <engine/input/input_types.h>

#include <array>
#include <cstddef>

namespace Engine
{
	class InputEventQueue
	{
	public:
		static constexpr size_t Capacity = 4096;

		InputEventQueue();

		void Push(const InputEvent& event);
		bool Pop(InputEvent& out_event);
		void Clear();

		bool IsEmpty() const { return m_head == m_tail; }
		size_t Size() const;
		bool Overflowed() const { return m_overflowed; }

	private:
		std::array<InputEvent, Capacity> m_events;
		size_t m_head = 0;
		size_t m_tail = 0;
		bool m_overflowed = false;
		uint64_t m_next_sequence = 1;
	};
}