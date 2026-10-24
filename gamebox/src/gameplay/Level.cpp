#include "Level.h"

std::span<const std::string_view> Level1Rows()
{
	static constexpr std::string_view rows[] = {
		"########################################################",   //  0
		"#......................................................#",   //  1
		"#......................................................#",   //  2
		"#......................................................#",   //  3
		"#......................................................#",   //  4
		"#......................................................#",   //  5
		"#...................................................CG.#",   //  6
		"#...................................................####",   //  7
		"#................................................C.....#",   //  8
		"#...............................................###....#",   //  9
		"#............................................C.........#",   // 10
		"#...........................................###........#",   // 11
		"#..........CCC.....C............C....C.................#",   // 12
		"#.P.........^...........#.....E..........##............#",   // 13
		"##################...###############...#################",   // 14
		"##################...###############...#################",   // 15
	};
	return rows;
}

bool ParseLevel(std::span<const std::string_view> rows, Level& out, std::string& error)
{
	out = Level{};
	error.clear();
	if (rows.empty())
	{
		error = "Level has no rows";
		return false;
	}

	const int width = static_cast<int>(rows[0].size());
	const int height = static_cast<int>(rows.size());
	int spawn_count = 0;

	for (int row = 0; row < height; ++row)
	{
		if (static_cast<int>(rows[row].size()) != width)
		{
			error = "row " + std::to_string(row) + " is " + std::to_string(rows[row].size()) + " wide, expected " + std::to_string(width);
			return false;
		}
		
		const float bottom = static_cast<float>(height - 1 - row);
		int run_start = -1;

		for (int col = 0; col <= width; ++col)
		{
			const char ch = (col < width) ? rows[row][col] : '.';  // one past the end flushes the last run

			if (ch == '#')
			{
				if (run_start < 0)
				{
					run_start = col;
				}
				continue;
			}
			if (run_start >= 0)
			{
				const float w = static_cast<float>(col - run_start);
				out.solids.push_back({ { run_start + w * 0.5f, bottom + 0.5f }, { w * 0.5f, 0.5f } });
				run_start = -1;
			}

			if (col == width)
			{
				break;
			}

			const float cx = static_cast<float>(col) + 0.5f;
			switch(ch)
			{
			case '.': 
			case ' ': 
				break;
			case 'P': 
				out.spawn_feet = { cx, bottom }; 
				++spawn_count; 
				break;
			case 'C': 
				out.pickups.push_back({ { cx, bottom + 0.5f }, false }); 
				break;
			case '^': 
				out.hazards.push_back({ { { cx, bottom + 0.25f }, { 0.4f, 0.25f } } }); 
				break;
			case 'G': 
				out.goals.push_back({ { { cx, bottom + 0.5f }, { 0.5f, 0.5f } } }); 
				break;
			case 'E':
			{
				Enemy e;
				e.body.position = { cx, bottom + e.body.half_size.y };
				out.enemies.push_back(e);
				break;
			}
			default:
				error = std::string("unknown character '") + ch + "' at row " + std::to_string(row) +
					", column " + std::to_string(col);
				return false;

			}
		}

	}

	if (spawn_count != 1)
	{
		error = "expected exactly one 'P', found " + std::to_string(spawn_count);
		return false;
	}

	if (out.goals.empty())
	{
		error = "level has no goal 'G'";
		return false;
	}

	out.bounds_min = { 0.0f, 0.0f };
	out.bounds_max = { static_cast<float>(width), static_cast<float>(height) };
	out.kill_y = out.bounds_min.y - 3.0f;

	return true;
}
