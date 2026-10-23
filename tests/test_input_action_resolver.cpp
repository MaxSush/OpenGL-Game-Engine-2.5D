#include <doctest/doctest.h>

#include "input/input_action_resolver.h"
#include "input/physical_input_state.h"
#include "test_helpers.h"

#include <vector>

using namespace Engine;
using namespace TestUtil;

namespace
{
	struct ResolverFixture
	{
		InputActionMap map;
		InputActionResolver resolver;
		PhysicalInputState state;
		std::vector<InputContextId> contexts;   // empty: context filtering is not implemented yet

		InputActionId Add(const char* name, InputActionType type)
		{
			return map.RegisterAction(name, type);
		}

		void BindKey(InputActionId action, Key key, float scale = 1.0f,
			InputVectorComponent comp = InputVectorComponent::X, ModifierKey mods = ModifierKey::None)
		{
			InputBinding b;
			b.action = action;
			b.source = InputBindingSource::Key;
			b.key = key;
			b.scale = scale;
			b.component = comp;
			b.required_mods = mods;
			map.AddBinding(b);
		}

		void Finish() // after all actions are registered
		{
			map.FinalizeActions(); 
			resolver.SetActionMap(&map);
		}

		void Frame()
		{
			state.BeginFrame();
		}

		void Resolve()
		{
			resolver.Resolve(state, contexts);
		}

		const ResolvedAction& Get(InputActionId id) const
		{
			const ResolvedAction* a = resolver.FindAction(id);
			REQUIRE(a != nullptr);
			return *a;
		}
	};
}

TEST_CASE_FIXTURE(ResolverFixture, "digital: down, pressed and released lifecycle")
{
	const auto jump = Add("Jump", InputActionType::Digital);
	BindKey(jump, Key::Space);
	Finish();

	Frame(); state.ApplyEvent(KeyDown(Key::Space)); 
	Resolve();
	CHECK(Get(jump).down);
	CHECK(Get(jump).pressed);
	CHECK_FALSE(Get(jump).released);

	Frame(); 
	Resolve();                                   // still held
	CHECK(Get(jump).down);
	CHECK_FALSE(Get(jump).pressed);

	Frame(); state.ApplyEvent(KeyUp(Key::Space)); 
	Resolve();
	CHECK_FALSE(Get(jump).down);
	CHECK(Get(jump).released);
	CHECK_FALSE(Get(jump).pressed);

	Frame(); 
	Resolve();
	CHECK_FALSE(Get(jump).released);
}

TEST_CASE_FIXTURE(ResolverFixture, "scalar: opposite keys cancel and scale is applied")
{
	const auto move = Add("MoveX", InputActionType::Scalar);
	BindKey(move, Key::A, -1.0f);
	BindKey(move, Key::D, 1.0f);
	Finish();

	Frame(); state.ApplyEvent(KeyDown(Key::D)); Resolve();
	CHECK(Get(move).scalar == doctest::Approx(1.0));
	CHECK(Get(move).down);

	Frame(); state.ApplyEvent(KeyDown(Key::A)); Resolve();      // both held
	CHECK(Get(move).scalar == doctest::Approx(0.0));
	CHECK_FALSE(Get(move).down);
	CHECK(Get(move).released);                                  // went from active to inactive

	Frame(); state.ApplyEvent(KeyUp(Key::D)); Resolve();
	CHECK(Get(move).scalar == doctest::Approx(-1.0));
}

TEST_CASE_FIXTURE(ResolverFixture, "scalar: values clamp to [-1, 1]")
{
	const auto move = Add("MoveX", InputActionType::Scalar);
	BindKey(move, Key::D, 2.5f);
	Finish();

	Frame(); state.ApplyEvent(KeyDown(Key::D)); Resolve();
	CHECK(Get(move).scalar == doctest::Approx(1.0));
}

TEST_CASE_FIXTURE(ResolverFixture, "vector2: components combine and diagonals are normalized")
{
	const auto mv = Add("Move", InputActionType::Vector2);
	BindKey(mv, Key::A, -1.0f, InputVectorComponent::X);
	BindKey(mv, Key::D, 1.0f, InputVectorComponent::X);
	BindKey(mv, Key::S, -1.0f, InputVectorComponent::Y);
	BindKey(mv, Key::W, 1.0f, InputVectorComponent::Y);
	Finish();

	Frame(); state.ApplyEvent(KeyDown(Key::D)); Resolve();
	CHECK(Get(mv).vector2.x == doctest::Approx(1.0));
	CHECK(Get(mv).vector2.y == doctest::Approx(0.0));

	Frame(); state.ApplyEvent(KeyDown(Key::W)); Resolve();      // D + W: |(1,1)| > 1
	CHECK(Get(mv).vector2.x == doctest::Approx(0.70710678));
	CHECK(Get(mv).vector2.y == doctest::Approx(0.70710678));
}

TEST_CASE_FIXTURE(ResolverFixture, "required modifiers gate a binding")
{
	const auto act = Add("Special", InputActionType::Digital);
	BindKey(act, Key::E, 1.0f, InputVectorComponent::X, ModifierKey::Shift);
	Finish();

	Frame(); state.ApplyEvent(KeyDown(Key::E)); Resolve();
	CHECK_FALSE(Get(act).down);                                 // no shift yet

	Frame(); state.ApplyEvent(KeyDown(Key::LeftShift)); Resolve();
	CHECK(Get(act).down);                                       // E is still held, shift now too
}

TEST_CASE_FIXTURE(ResolverFixture, "mouse axis binding scales the frame delta")
{
	const auto look = Add("LookX", InputActionType::Scalar);
	InputBinding b;
	b.action = look;
	b.source = InputBindingSource::MouseAxisX;
	b.scale = 0.1f;
	map.AddBinding(b);
	Finish();

	Frame(); state.ApplyEvent(MouseMove(0.0, 0.0)); Resolve();  // first move: no delta
	CHECK(Get(look).scalar == doctest::Approx(0.0));

	Frame(); state.ApplyEvent(MouseMove(5.0, 0.0)); Resolve();
	CHECK(Get(look).scalar == doctest::Approx(0.5));
}

TEST_CASE_FIXTURE(ResolverFixture, "unknown action ids do not crash or disturb other actions")
{
	const auto real = Add("Real", InputActionType::Digital);
	BindKey(real, Key::A);

	InputBinding stray;
	stray.action = InputActionId{ 999 };
	stray.source = InputBindingSource::Key;
	stray.key = Key::A;
	map.AddBinding(stray);
	Finish();

	Frame(); state.ApplyEvent(KeyDown(Key::A)); Resolve();
	CHECK(Get(real).down);
	CHECK(resolver.FindAction(InputActionId{ 999 }) == nullptr);
	CHECK(resolver.FindAction(InputActionId{}) == nullptr);     // default id is invalid
}

TEST_CASE_FIXTURE(ResolverFixture, "no active binding gives neutral actions")
{
	const auto d = Add("D", InputActionType::Digital);
	const auto s = Add("S", InputActionType::Scalar);
	const auto v = Add("V", InputActionType::Vector2);
	BindKey(d, Key::A);
	BindKey(s, Key::A);
	BindKey(v, Key::A);
	Finish();

	Frame(); Resolve();
	CHECK_FALSE(Get(d).down);
	CHECK(Get(s).scalar == doctest::Approx(0.0));
	CHECK(Get(v).vector2.x == doctest::Approx(0.0));
	CHECK_FALSE(Get(v).down);
}

// Deferred: Resolve() accepts active_contexts but ignores it (the accepted C4100 warning).
// Skipped on purpose so the gap stays visible in test output. Unskip when contexts are wired
// in (trigger: pause menu or ImGui capture, see the deferred register in STATUS.md).
TEST_CASE("resolver: bindings are filtered by active context" * doctest::skip())
{
	FAIL("context filtering is not implemented");
}

// --- InputActionMap (compiled in anyway, cheap to cover) ---

TEST_CASE("action map: ids are sequential and names are unique")
{
	InputActionMap m;
	const auto a = m.RegisterAction("A", InputActionType::Digital);
	const auto b = m.RegisterAction("B", InputActionType::Scalar);
	CHECK(a.value == 0u);
	CHECK(b.value == 1u);
	CHECK(m.RegisterAction("A", InputActionType::Digital) == a);   // duplicate name returns the same id
	CHECK(m.FindAction("B") == b);
	CHECK_FALSE(m.FindAction("Nope").IsValid());
}

TEST_CASE("action map: no new actions after finalize, existing names still resolve")
{
	InputActionMap m;
	const auto a = m.RegisterAction("A", InputActionType::Digital);
	m.FinalizeActions();
	CHECK(m.AreActionsFinalized());
	CHECK_FALSE(m.RegisterAction("Late", InputActionType::Digital).IsValid());
	CHECK(m.RegisterAction("A", InputActionType::Digital) == a);
}