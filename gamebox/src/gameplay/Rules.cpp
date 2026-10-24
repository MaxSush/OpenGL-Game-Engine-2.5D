#include "Rules.h"

void ResolveInteractions(const Player& player, Level& level, Progress& progress)
{
	if (progress.state != GameState::PLAYING)
	{
		return;
	}

	const AABB me = BoundsOf(player.body);

	for (Pickup& coin : level.pickups)
	{
		if (!coin.collected && Overlaps(me, AABB{ coin.position, {kPickupHalf, kPickupHalf} }))
		{
			coin.collected = true;
			++progress.coins;
		}
	}

	auto die = [&](DeathCause cause)
		{
			progress.state = GameState::DEAD;
			progress.cause = cause;
		};

	for (const Hazard& h : level.hazards)
	{
		if (Overlaps(me, h.box))
		{
			die(DeathCause::HAZARD);
			return;
		}
	}

	for (const Enemy& e : level.enemies)
	{
		if (Overlaps(me, BoundsOf(e.body)))
		{
			die(DeathCause::ENEMY);
			return;
		}
	}

	const glm::vec2 p = player.body.position;
	if (p.y < level.kill_y || p.x < level.bounds_min.x || p.x > level.bounds_max.x)
	{
		die(DeathCause::FALL);
		return;
	}

	for (const Goal& g : level.goals)
	{
		if (Overlaps(me, g.box))
		{
			progress.state = GameState::WIN;
			return;
		}
	}
}
