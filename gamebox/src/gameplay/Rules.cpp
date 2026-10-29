#include "Rules.h"

#include "Collision.h"
#include <engine/gameplay/scene.h>

void ResolveInteractions(Engine::Scene& scene, Level& level, Progress& progress)
{
	if (progress.state != GameState::PLAYING)
	{
		return;
	}

	scene.Each<Player>([&](Engine::Entity, Player& player)
		{
			const Body me = BoundsOf(player.transform);
			auto die = [&](DeathCause cause)
				{
					progress.state = GameState::DEAD;
					progress.cause = cause;
				};

			scene.Each<Pickup>([&](Engine::Entity coin, Pickup& pk)
				{
					if (Overlaps(me, Body{ pk.position, {kPickupHalf, kPickupHalf} }))
					{
						++progress.coins;
						coin.Destroy();
					}
				});

			scene.Each<Hazard>([&](Engine::Entity, Hazard& h)
				{
					if (progress.state == GameState::PLAYING && Overlaps(me, h.b))
					{
						die(DeathCause::HAZARD);
						return;
					}
				});

			scene.Each<Enemy>([&](Engine::Entity, Enemy& e)
				{
					if (progress.state == GameState::PLAYING && Overlaps(me, BoundsOf(e.transform)))
					{
						die(DeathCause::ENEMY);
						return;
					}
				});
			
			const glm::vec2 p = player.transform.position;
			if (progress.state == GameState::PLAYING)
			{
				if (p.y < level.kill_y || p.x < level.bounds_min.x || p.x > level.bounds_max.x)
				{
					die(DeathCause::FALL);
					return;
				}
			}

			scene.Each<Goal>([&](Engine::Entity, Goal& g)
				{
					if (progress.state == GameState::PLAYING && Overlaps(me, g.b))
					{
						progress.state = GameState::WIN;
						return;
					}
				});
		});
}
