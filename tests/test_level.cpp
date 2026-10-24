#include <doctest/doctest.h>

#include <gameplay/Level.h>

#include <string_view>
#include <string>
#include <span>

TEST_CASE("level: row 0 is the top and '#' runs merge")
{
	const std::string_view rows[] = { "P.G", "###" };
	Level lv; 
	std::string err;
	REQUIRE(ParseLevel(rows, lv, err));
	REQUIRE(lv.solids.size() == 1);
	CHECK(lv.solids[0].half.x == doctest::Approx(1.5));
	CHECK(lv.solids[0].center.y == doctest::Approx(0.5));
	CHECK(lv.spawn_feet.y == doctest::Approx(1.0));      // bottom edge of the top row
}

TEST_CASE("level: Level 1 parses and nothing starts inside a solid")
{
	Level lv; 
	std::string err;
	REQUIRE(ParseLevel(Level1Rows(), lv, err));
	CHECK(lv.pickups.size() == 9);
	CHECK(lv.hazards.size() == 1);
	CHECK(lv.goals.size() == 1);
	//CHECK(lv.enemies.size() == 1);
	// spawn, enemies, goals and pickups must not overlap any solid
}