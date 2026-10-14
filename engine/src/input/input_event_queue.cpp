#include "input_event_queue.h"

namespace Engine
{
	InputEventQueue::InputEventQueue() = default;

	void InputEventQueue::Push(const InputEvent& event)
	{
		InputEvent e = event;
		e.sequence = m_next_sequence++;

		size_t next_tail = (m_tail + 1) % Capacity;
		if (next_tail == m_head)
		{
			m_overflowed = true;
			m_head = (m_head + 1) % Capacity;
		}

		m_events[m_tail] = e;
		m_tail = next_tail;
	}

	bool InputEventQueue::Pop(InputEvent& out_event)
	{
		if (m_head == m_tail)
		{
			return false;
		}

		out_event = m_events[m_head];
		m_head = (m_head + 1) % Capacity;
		return true;
	}

	void InputEventQueue::Clear()
	{
		m_head = 0;
		m_tail = 0;
		m_overflowed = false;
	}

	size_t InputEventQueue::Size() const
	{
		if (m_tail >= m_head)
		{
			return m_tail - m_head;
		}
		return Capacity - m_head + m_tail;
	}
}