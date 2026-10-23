#include <doctest/doctest.h>

#include "input/input_event_queue.h"
#include "test_helpers.h"

#include <memory>

using namespace Engine;
using namespace TestUtil;

static std::unique_ptr<InputEventQueue> MakeQueue() 
{ 
	return std::make_unique<InputEventQueue>(); 
}

TEST_CASE("queue: new queue is empty")
{
	auto q = MakeQueue();
	InputEvent out;
	CHECK(q->IsEmpty());
	CHECK(q->Size() == 0u);
	CHECK_FALSE(q->Overflowed());
	CHECK_FALSE(q->Pop(out));
}

TEST_CASE("queue: type and payload are preserved")
{
	auto q = MakeQueue();
	q->Push(KeyDown(Key::A));

	InputEvent out;
	REQUIRE(q->Pop(out));
	CHECK(out.type == InputEventType::KeyDown);
	const KeyEvent* k = out.AsKey();
	REQUIRE(k != nullptr);
	CHECK(k->key == Key::A);
	CHECK(out.AsMouseMove() == nullptr);
}

TEST_CASE("queue: events pop in FIFO order and Size tracks pushes and pops")
{
	auto q = MakeQueue();
	q->Push(KeyDown(Key::A));
	q->Push(KeyDown(Key::B));
	q->Push(KeyDown(Key::C));
	CHECK(q->Size() == 3u);

	InputEvent out;
	REQUIRE(q->Pop(out)); CHECK(out.AsKey()->key == Key::A);
	CHECK(q->Size() == 2u);
	REQUIRE(q->Pop(out)); CHECK(out.AsKey()->key == Key::B);
	REQUIRE(q->Pop(out)); CHECK(out.AsKey()->key == Key::C);
	CHECK(q->IsEmpty());
}

TEST_CASE("queue: sequence numbers start at 1 and increase")
{
	auto q = MakeQueue();
	q->Push(KeyDown(Key::A));
	q->Push(KeyDown(Key::A));
	q->Push(KeyDown(Key::A));

	InputEvent out;
	uint64_t previous = 0;
	while (q->Pop(out))
	{
		CHECK(out.sequence > previous);
		previous = out.sequence;
	}
	CHECK(previous == 3u);
}

TEST_CASE("queue: Clear empties the queue but does not reset the sequence counter")
{
	auto q = MakeQueue();
	q->Push(KeyDown(Key::A));
	q->Push(KeyDown(Key::A));
	q->Clear();

	CHECK(q->IsEmpty());
	CHECK_FALSE(q->Overflowed());

	q->Push(KeyDown(Key::A));
	InputEvent out;
	REQUIRE(q->Pop(out));
	CHECK(out.sequence == 3u);   // current behaviour: the counter keeps counting
}

TEST_CASE("queue: usable capacity is Capacity - 1 and overflow drops the oldest event")
{
	auto q = MakeQueue();
	const std::size_t cap = InputEventQueue::Capacity;

	for (std::size_t i = 0; i < cap - 1; ++i)
	{
		q->Push(Scroll(static_cast<double>(i), 0.0));
	}
	CHECK(q->Size() == cap - 1);
	CHECK_FALSE(q->Overflowed());

	q->Push(Scroll(static_cast<double>(cap - 1), 0.0));   // one too many
	CHECK(q->Overflowed());
	CHECK(q->Size() == cap - 1);

	InputEvent out;
	REQUIRE(q->Pop(out));
	CHECK(out.sequence == 2u);   // sequence 1 was dropped
	uint64_t last = out.sequence;
	while (q->Pop(out))
	{
		CHECK(out.sequence == last + 1);   // the survivors are still contiguous and in order
		last = out.sequence;
	}
	CHECK(last == cap);                    // newest event survived
}

TEST_CASE("queue: indices wrap around without overflow")
{
	auto q = MakeQueue();
	InputEvent out;
	uint64_t previous = 0;
	for (std::size_t i = 0; i < InputEventQueue::Capacity * 3; ++i)
	{
		q->Push(KeyDown(Key::A));
		REQUIRE(q->Pop(out));
		CHECK(out.sequence > previous);
		previous = out.sequence;
	}
	CHECK(q->IsEmpty());
	CHECK_FALSE(q->Overflowed());
}