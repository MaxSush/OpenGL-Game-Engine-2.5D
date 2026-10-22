#include "Level.h"

Level BuildLevel()
{
	Level level;

	auto add = [&](float cx, float cy, float w, float h)
		{
			level.solids.push_back({ {cx,cy}, {w * 0.5f, h * 0.5f} });
		};

	// Floor: top at y = -5.0
	add(0.0f, -5.5f, 20.0f, 1.0f);

	// Side walls (inner edges at x = +/-10)
	add(-10.5f, 0.0f, 1.0f, 12.0f);
	add(10.5f, 0.0f, 1.0f, 12.0f);

	// A low step on the floor (top at y = -4.0)
	add(3.0f, -4.5f, 1.5f, 1.0f);

	// Platforms, each about 2.0 units above the previous top. Full jump height is 2.4.
	add(-5.0f, -3.25f, 4.0f, 0.5f);   // top -3.0
	add(0.0f, -1.25f, 4.0f, 0.5f);    // top -1.0
	add(5.0f, 0.75f, 4.0f, 0.5f);     // top  1.0
	add(0.0f, 2.75f, 4.0f, 0.5f);     // top  3.0

	// Standing on the floor: y = floor top (-5.0) + player half height (0.5)
	level.spawn = { -8.0f, -4.5f };
	return level;
}
