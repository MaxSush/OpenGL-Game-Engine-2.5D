#include <doctest/doctest.h>

#include "input/physical_input_state.h"
#include "test_helpers.h"

using namespace Engine;
using namespace TestUtil;

TEST_CASE("physical: key down sets down and pressed")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(KeyDown(Key::A));
	CHECK(s.IsKeyDown(Key::A));
	CHECK(s.IsKeyPressed(Key::A));
	CHECK_FALSE(s.IsKeyReleased(Key::A));
}

TEST_CASE("physical: a second key down while held is not a new press")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(KeyDown(Key::A));
	s.BeginFrame();
	s.ApplyEvent(KeyDown(Key::A));
	CHECK(s.IsKeyDown(Key::A));
	CHECK_FALSE(s.IsKeyPressed(Key::A));
}

TEST_CASE("physical: KeyRepeat events are ignored")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(MakeKey(InputEventType::KeyRepeat, Key::A));
	CHECK_FALSE(s.IsKeyDown(Key::A));
	CHECK_FALSE(s.IsKeyPressed(Key::A));
}

TEST_CASE("physical: key up clears down and sets released")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(KeyDown(Key::A));
	s.BeginFrame();
	s.ApplyEvent(KeyUp(Key::A));
	CHECK_FALSE(s.IsKeyDown(Key::A));
	CHECK(s.IsKeyReleased(Key::A));
	CHECK_FALSE(s.IsKeyPressed(Key::A));
}

TEST_CASE("physical: BeginFrame clears transitions but keeps held keys")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(KeyDown(Key::A));
	s.BeginFrame();
	CHECK(s.IsKeyDown(Key::A));
	CHECK_FALSE(s.IsKeyPressed(Key::A));
	CHECK_FALSE(s.IsKeyReleased(Key::A));
}

TEST_CASE("physical: press and release in one frame keeps both transitions")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(KeyDown(Key::Space));
	s.ApplyEvent(KeyUp(Key::Space));
	CHECK_FALSE(s.IsKeyDown(Key::Space));
	CHECK(s.IsKeyPressed(Key::Space));
	CHECK(s.IsKeyReleased(Key::Space));
}

TEST_CASE("physical: mouse buttons mirror key transitions")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(MouseDown(MouseButton::Left));
	CHECK(s.IsMouseButtonDown(MouseButton::Left));
	CHECK(s.IsMouseButtonPressed(MouseButton::Left));

	s.BeginFrame();
	CHECK(s.IsMouseButtonDown(MouseButton::Left));
	CHECK_FALSE(s.IsMouseButtonPressed(MouseButton::Left));

	s.ApplyEvent(MouseUp(MouseButton::Left));
	CHECK_FALSE(s.IsMouseButtonDown(MouseButton::Left));
	CHECK(s.IsMouseButtonReleased(MouseButton::Left));
}

TEST_CASE("physical: first mouse move has no delta, later moves accumulate")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(MouseMove(100.0, 50.0));
	CHECK(s.GetMousePosition().x == doctest::Approx(100.0));
	CHECK(s.GetMouseDelta().x == doctest::Approx(0.0));

	s.BeginFrame();
	s.ApplyEvent(MouseMove(110.0, 50.0));
	s.ApplyEvent(MouseMove(115.0, 60.0));
	CHECK(s.GetMouseDelta().x == doctest::Approx(15.0));
	CHECK(s.GetMouseDelta().y == doctest::Approx(10.0));

	s.BeginFrame();
	CHECK(s.GetMouseDelta().x == doctest::Approx(0.0));
	CHECK(s.GetMousePosition().x == doctest::Approx(115.0));   // position persists
}

TEST_CASE("physical: no delta spike after focus loss or gain")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(MouseMove(100.0, 100.0));

	s.BeginFrame();
	s.ApplyEvent(Focus(false));
	s.ApplyEvent(Focus(true));
	s.ApplyEvent(MouseMove(500.0, 400.0));
	CHECK(s.GetMouseDelta().x == doctest::Approx(0.0));
	CHECK(s.GetMouseDelta().y == doctest::Approx(0.0));
	CHECK(s.GetMousePosition().x == doctest::Approx(500.0));
}

TEST_CASE("physical: scroll accumulates within a frame and resets")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(Scroll(0.0, 1.0));
	s.ApplyEvent(Scroll(0.0, 2.0));
	CHECK(s.GetScrollDelta().y == doctest::Approx(3.0));
	s.BeginFrame();
	CHECK(s.GetScrollDelta().y == doctest::Approx(0.0));
}

TEST_CASE("physical: focus loss releases everything")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(KeyDown(Key::A));
	s.ApplyEvent(MouseDown(MouseButton::Left));
	CHECK(s.HasFocus());

	s.BeginFrame();
	s.ApplyEvent(Focus(false));
	CHECK_FALSE(s.HasFocus());
	CHECK_FALSE(s.IsKeyDown(Key::A));
	CHECK(s.IsKeyReleased(Key::A));
	CHECK_FALSE(s.IsMouseButtonDown(MouseButton::Left));
	CHECK(s.IsMouseButtonReleased(MouseButton::Left));
}

TEST_CASE("physical: modifiers follow modifier key state")
{
	PhysicalInputState s;
	s.BeginFrame();
	s.ApplyEvent(KeyDown(Key::LeftShift));
	CHECK(HasModifier(s.GetModifiers(), ModifierKey::Shift));
	CHECK_FALSE(HasModifier(s.GetModifiers(), ModifierKey::Control));

	s.ApplyEvent(KeyUp(Key::LeftShift));
	CHECK(s.GetModifiers() == ModifierKey::None);
}

TEST_CASE("physical: invalid keys and buttons are handled safely")
{
	PhysicalInputState s;
	s.BeginFrame();

	s.ApplyEvent(KeyDown(Key::Unknown));
	s.ApplyEvent(KeyDown(static_cast<Key>(9999)));
	s.ApplyEvent(MouseDown(MouseButton::None));
	s.ApplyEvent(MouseDown(static_cast<MouseButton>(99)));

	CHECK_FALSE(s.IsKeyDown(Key::Unknown));
	CHECK_FALSE(s.IsKeyDown(Key::Count));
	CHECK_FALSE(s.IsKeyDown(static_cast<Key>(9999)));
	CHECK_FALSE(s.IsKeyPressed(Key::Unknown));
	CHECK_FALSE(s.IsMouseButtonDown(MouseButton::None));
	CHECK_FALSE(s.IsMouseButtonDown(MouseButton::Count));

	InputEvent no_payload;                          // payload is monostate
	no_payload.type = InputEventType::KeyDown;
	s.ApplyEvent(no_payload);                       // must not crash
	CHECK_FALSE(s.IsKeyDown(Key::A));
}