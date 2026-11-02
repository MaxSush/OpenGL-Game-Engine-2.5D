#include "Rules.h"

#include <engine/gameplay/scene.h>

#include <engine/physics/collision.h>
#include "Components.h"
#include "Level.h"

void ResolveInteractions(Engine::Scene& scene, const Level& level, Progress& progress)
{
	if (progress.state != GameState::PLAYING)
	{
		return;
	}

	scene.Each<Player, Engine::Transform, Engine::Collider>([&](Engine::Entity, 
		Player&, Engine::Transform& tr, Engine::Collider& col)
		{
			const Engine::StaticBox me = BoundsOf(tr, col);
			auto die = [&](DeathCause cause)
				{
					progress.state = GameState::DEAD;
					progress.cause = cause;
				};

			scene.Each<Pickup>([&](Engine::Entity coin, Pickup& pk)
				{
					if (Engine::Overlaps(me, Engine::StaticBox{ pk.position, {kPickupHalf, kPickupHalf} }))
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

			scene.Each<Enemy, Engine::Transform, Engine::Collider>(
				[&](Engine::Entity, Enemy&, Engine::Transform& tr, Engine::Collider& col)
				{
					if (progress.state == GameState::PLAYING && Overlaps(me, Engine::BoundsOf(tr, col)))
					{
						die(DeathCause::ENEMY);
						return;
					}
				});

			const glm::vec2 pos = tr.position;
			if (progress.state == GameState::PLAYING)
			{
				if (pos.y < level.kill_y || pos.x < level.bounds_min.x || pos.x > level.bounds_max.x)
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
