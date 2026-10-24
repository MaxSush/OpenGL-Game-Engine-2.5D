#include <doctest/doctest.h>

#include "gameplay/Player.h"

#include <algorithm>
#include <vector>

namespace
{
	constexpr float kDt = 1.0f / 60.0f;

	std::vector<AABB> Floor() { return { AABB{ { 0.0f, -0.5f }, { 20.0f, 0.5f } } }; } // top at y = 0

	void Step(Player& p, PlayerIntent& in, const Tuning& t, const std::vector<AABB>& solids, float dt = kDt)
	{
	    StepPlayer(p, in, t, solids, dt);
	}

	// A player standing on the floor after a few settling steps, with grounded = true.
	Player Settled(const Tuning& t, const std::vector<AABB>& solids, float dt = kDt)
	{
	    Player p;

	    p.body.position = { 0.0f, 0.5f };
	    PlayerIntent none;
	    for (int i = 0; i < 3; ++i) Step(p, none, t, solids, dt);
	    REQUIRE(p.body.grounded);
	    return p;
	}

	Player Airborne(float y, float vy = 0.0f)
	{
	    Player p;
		p.body.position = { 0.0f, y };
		p.body.velocity = { 0.0f, vy };
	    return p;
	}
}

TEST_CASE("player: gravity pulls down and a standing player stays grounded")
{
	const Tuning t;
	const auto floor = Floor();
	Player p = Settled(t, floor);
	PlayerIntent none;
	for (int i = 0; i < 30; ++i)
	{
		Step(p, none, t, floor);
		CHECK(p.body.grounded);
		CHECK(p.body.position.y == doctest::Approx(0.5));
	}
}

TEST_CASE("player: one airborne step applies gravity in the negative Y direction")
{
	const Tuning t;
	Player p = Airborne(10.0f);
	PlayerIntent none;
	Step(p, none, t, {});
	CHECK(p.body.velocity.y == doctest::Approx(t.gravity * kDt));
	CHECK(p.body.velocity.y < 0.0f);
}

TEST_CASE("collision: MoveResult reports which axis was blocked")
{
	const AABB wall{ { 5.0f, 0.0f }, { 0.5f, 5.0f } };
	Body b;
	b.half_size = { 0.4f, 0.5f };
	b.position = { 4.0f, 0.0f };
	b.velocity = { 10.0f, 0.0f };
	const MoveResult r = MoveAndCollide(b, { wall }, 0.1f);
	CHECK(r.hit_x);
	CHECK_FALSE(r.hit_y);
}

TEST_CASE("player: fall speed is capped")
{
	const Tuning t;
	Player p = Airborne(1000.0f, -100.0f);
	PlayerIntent none;
	Step(p, none, t, {});
	CHECK(p.body.velocity.y == doctest::Approx(t.max_fall_speed));
}

TEST_CASE("player: landing sets grounded and zeroes vertical velocity")
{
	const Tuning t;
	const auto floor = Floor();
	Player p = Airborne(2.0f);
	PlayerIntent none;
	bool landed = false;
	for (int i = 0; i < 240 && !landed; ++i)
	{
		Step(p, none, t, floor);
		landed = p.body.grounded;
	}
	REQUIRE(landed);
	CHECK(p.body.velocity.y == doctest::Approx(0.0));
	CHECK(p.body.position.y == doctest::Approx(0.5));
}

TEST_CASE("player: horizontal input accelerates toward max speed without overshoot")
{
	const Tuning t;
	const auto floor = Floor();
	Player p = Settled(t, floor);
	PlayerIntent in;
	in.move_x = 1.0f;

	Step(p, in, t, floor);
	CHECK(p.body.velocity.x == doctest::Approx(t.accel * kDt));

	for (int i = 0; i < 120; ++i)
	{
		Step(p, in, t, floor);
		CHECK(p.body.velocity.x <= t.max_speed + 0.0001f);
	}
	CHECK(p.body.velocity.x == doctest::Approx(t.max_speed));
}

TEST_CASE("player: releasing input applies friction down to exactly zero")
{
	const Tuning t;
	const auto floor = Floor();
	Player p = Settled(t, floor);
	p.body.velocity.x = t.max_speed;
	PlayerIntent none;

	float previous = p.body.velocity.x;
	for (int i = 0; i < 100 && p.body.velocity.x != 0.0f; ++i)
	{
		Step(p, none, t, floor);
		CHECK(p.body.velocity.x <= previous);
		CHECK(p.body.velocity.x >= 0.0f);                 // friction never reverses direction
		previous = p.body.velocity.x;
	}
	CHECK(p.body.velocity.x == 0.0f);
}

TEST_CASE("player: a grounded jump sets upward velocity and consumes the press")
{
	const Tuning t;
	const auto floor = Floor();
	Player p = Settled(t, floor);
	PlayerIntent in;
	in.jump_pressed = true;
	in.jump_held = true;

	Step(p, in, t, floor);
	CHECK(p.body.velocity.y == doctest::Approx(t.jump_velocity + t.gravity * kDt));
	CHECK_FALSE(p.body.grounded);
	CHECK_FALSE(in.jump_pressed);                    // consumed by StepPlayer
}

TEST_CASE("player: no jump in the air without coyote time")
{
	const Tuning t;
	Player p = Airborne(10.0f);
	PlayerIntent in;
	in.jump_pressed = true;
	in.jump_held = true;
	Step(p, in, t, {});
	CHECK(p.body.velocity.y == doctest::Approx(t.gravity * kDt));
}

TEST_CASE("player: coyote time allows a late jump but only inside the window")
{
	const Tuning t;

	Player inside = Airborne(10.0f);
	inside.coyote_timer = t.coyote_time * 0.5f;
	PlayerIntent in1; in1.jump_pressed = true; in1.jump_held = true;
	Step(inside, in1, t, {});
	CHECK(inside.body.velocity.y > 0.0f);

	Player outside = Airborne(10.0f);
	outside.coyote_timer = kDt * 0.5f;               // expires within this step
	PlayerIntent in2; in2.jump_pressed = true; in2.jump_held = true;
	Step(outside, in2, t, {});
	CHECK(outside.body.velocity.y < 0.0f);
}

TEST_CASE("player: a jump pressed just before landing is buffered")
{
	const Tuning t;
	REQUIRE(t.jump_buffer_time > kDt);
	const auto floor = Floor();

	Player p = Airborne(0.55f, -6.0f);               // lands during the first step
	PlayerIntent in;
	in.jump_pressed = true;
	in.jump_held = true;

	Step(p, in, t, floor);                           // press while airborne: lands, no jump yet
	REQUIRE(p.body.grounded);
	CHECK(p.body.velocity.y == doctest::Approx(0.0));

	Step(p, in, t, floor);                           // next step: buffer + grounded => jump
	CHECK(p.body.velocity.y > 0.0f);
}

TEST_CASE("player: releasing jump while rising applies extra gravity")
{
	const Tuning t;

	Player held = Airborne(10.0f, 10.0f);
	PlayerIntent h; h.jump_held = true;
	Step(held, h, t, {});

	Player released = Airborne(10.0f, 10.0f);
	PlayerIntent r; r.jump_held = false;
	Step(released, r, t, {});

	CHECK(held.body.velocity.y == doctest::Approx(10.0f + t.gravity * kDt));
	CHECK(released.body.velocity.y == doctest::Approx(10.0f + t.gravity * t.release_gravity_mult * kDt));
	CHECK(released.body.velocity.y < held.body.velocity.y);
}

namespace
{
	float MeasureApex(float dt)
	{
		const Tuning t;
		const auto floor = Floor();
		Player p = Settled(t, floor, dt);
		const float start = p.body.position.y;

		PlayerIntent in;
		in.jump_pressed = true;
		in.jump_held = true;

		float apex = start;
		for (int i = 0; i < 600; ++i)
		{
			Step(p, in, t, floor, dt);
			apex = std::max(apex, p.body.position.y);
			if (i > 0 && p.body.grounded) break;
		}
		return apex - start;
	}
}

TEST_CASE("player: full jump height matches the level design")
{
	// Level.cpp spaces platforms 2.0 apart and its comment claims a 2.4 jump. With semi-implicit
	// Euler at 1/60 s the discrete apex is about 2.3 (computed by hand: 138/60). The hard
	// requirement is that the jump clears a 2.0 gap and never exceeds the analytic value.
	const Tuning t;
	const float analytic = (t.jump_velocity * t.jump_velocity) / (2.0f * -t.gravity);

	const float apex = MeasureApex(kDt);
	CHECK(apex > 2.0f);
	CHECK(apex <= analytic + 0.001f);
	CHECK(apex == doctest::Approx(2.3).epsilon(0.03));   // if this fails, report the value
}

TEST_CASE("player: apex height is nearly independent of the step size")
{
	// Informational: the engine keeps a fixed 1/60 step, so this only guards against
	// changing fixed_step later without noticing the jump height shifts.
	CHECK(MeasureApex(1.0f / 120.0f) == doctest::Approx(MeasureApex(1.0f / 60.0f)).epsilon(0.05));
}