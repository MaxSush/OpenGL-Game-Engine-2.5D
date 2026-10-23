#include <doctest/doctest.h>

#include "gameplay/Player.h"   // includes Collision.h

namespace
{
	constexpr float kDt = 1.0f / 60.0f;

	Player MakePlayer(float x, float y, float vx = 0.0f, float vy = 0.0f)
	{
		Player p;
		p.position = { x, y };
		p.velocity = { vx, vy };
		return p;
	}

	// Floor top at y = 0
	AABB FloorTile(float cx, float half_w) 
	{ 
		return AABB{ { cx, -0.5f }, { half_w, 0.5f } }; 
	}
}

TEST_CASE("collision: non-overlapping solids do not affect movement")
{
	Player p = MakePlayer(0.0f, 10.0f, 3.0f, 0.0f);
	MoveAndCollide(p, { FloorTile(0.0f, 20.0f) }, 0.1f);
	CHECK(p.position.x == doctest::Approx(0.3));
	CHECK(p.position.y == doctest::Approx(10.0));
	CHECK(p.velocity.x == doctest::Approx(3.0));
	CHECK_FALSE(p.grounded);
}

TEST_CASE("collision: a falling player lands on top without penetrating")
{
	Player p = MakePlayer(0.0f, 1.0f, 0.0f, -10.0f);
	MoveAndCollide(p, { FloorTile(0.0f, 20.0f) }, 0.1f);       // would end at y = 0 (inside the floor)
	CHECK(p.position.y == doctest::Approx(0.5));                // bottom edge exactly on the floor top
	CHECK(p.velocity.y == doctest::Approx(0.0));
	CHECK(p.grounded);
}

TEST_CASE("collision: resting with zero velocity is not grounded (edges only touch)")
{
	// Documents the skin behaviour: grounded comes from being pushed into the floor each step,
	// which StepPlayer does through gravity. See test_player.cpp.
	Player p = MakePlayer(0.0f, 0.5f);
	MoveAndCollide(p, { FloorTile(0.0f, 20.0f) }, kDt);
	CHECK_FALSE(p.grounded);
	CHECK(p.position.y == doctest::Approx(0.5));
}

TEST_CASE("collision: moving into a wall stops on its face")
{
	const AABB wall{ { 5.0f, 0.0f }, { 0.5f, 5.0f } };

	Player right = MakePlayer(4.0f, 0.0f, 10.0f, 0.0f);
	MoveAndCollide(right, { wall }, 0.1f);
	CHECK(right.position.x == doctest::Approx(5.0 - 0.5 - 0.4)); // wall left face minus half width
	CHECK(right.velocity.x == doctest::Approx(0.0));

	Player left = MakePlayer(6.0f, 0.0f, -10.0f, 0.0f);
	MoveAndCollide(left, { wall }, 0.1f);
	CHECK(left.position.x == doctest::Approx(5.0 + 0.5 + 0.4));
	CHECK(left.velocity.x == doctest::Approx(0.0));
}

TEST_CASE("collision: jumping into a ceiling stops the rise")
{
	const AABB ceiling{ { 0.0f, 3.0f }, { 5.0f, 0.5f } };       // underside at y = 2.5
	Player p = MakePlayer(0.0f, 1.9f, 0.0f, 10.0f);
	MoveAndCollide(p, { ceiling }, 0.1f);
	CHECK(p.position.y == doctest::Approx(3.0 - 0.5 - 0.5));    // top edge on the underside
	CHECK(p.velocity.y == doctest::Approx(0.0));
	CHECK_FALSE(p.grounded);
}

TEST_CASE("collision: adjacent floor tiles leave no gap or penetration")
{
	const std::vector<AABB> tiles{ FloorTile(-1.0f, 1.0f), FloorTile(1.0f, 1.0f) };
	Player p = MakePlayer(0.0f, 1.0f, 0.0f, -10.0f);            // straddles the seam
	MoveAndCollide(p, tiles, 0.1f);
	CHECK(p.position.y == doctest::Approx(0.5));
	CHECK(p.grounded);
}

TEST_CASE("collision: sliding across a tile seam is not snagged")
{
	const std::vector<AABB> tiles{ FloorTile(-1.0f, 1.0f), FloorTile(1.0f, 1.0f) };
	Player p = MakePlayer(-0.2f, 0.5f, 5.0f, 0.0f);             // standing on the floor, running right
	MoveAndCollide(p, tiles, 0.1f);
	CHECK(p.position.x == doctest::Approx(0.3));
	CHECK(p.velocity.x == doctest::Approx(5.0));
}

TEST_CASE("collision: max fall speed cannot tunnel through a 0.5 thick platform")
{
	// Design constraint from the plan: fall speed * step must stay below platform thickness.
	// Level.cpp uses 0.5 thick platforms and Tuning::max_fall_speed is -25 at 1/60 s.
	const AABB platform{ { 0.0f, -0.25f }, { 5.0f, 0.25f } };    // top at y = 0, thickness 0.5
	const float fall = 25.0f;
	for (float start = 0.5f; start < 0.5f + fall * kDt; start += 0.01f)
	{
		Player p = MakePlayer(0.0f, start, 0.0f, -fall);
		MoveAndCollide(p, { platform }, kDt);
		CHECK(p.position.y >= doctest::Approx(0.5).epsilon(0.001));   // never ends below the top
	}
}