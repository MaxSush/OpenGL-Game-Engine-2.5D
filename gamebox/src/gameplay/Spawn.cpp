#include "Spawn.h"

#include <engine/gameplay/scene.h>

#include "Components.h"

namespace
{
	const glm::vec2 kActorHalf{ 0.5f, 0.5f };
	const glm::vec2 kSpikeHalf{ 0.4f, 0.25f };
	const glm::vec2 kGoalHalf{ 0.5f, 0.5f };

	Engine::Entity SpawnActor(Engine::Scene& scene, const char* name, glm::vec2 feet)
	{
		Engine::Entity e = scene.CreateEntity(name);
		e.AddComponent<Engine::Transform>(glm::vec2{ feet.x, feet.y + kActorHalf.y });
		e.AddComponent<Engine::Collider>(Engine::Collider{ kActorHalf });
		e.AddComponent<Engine::Motion>();
		return e;
	}
}


SpawnResult SpawnLevel(Engine::Scene& scene, const Level& level)
{
	SpawnResult result;
	for (const SpawnPoint& s : level.spawns)
	{
		switch (s.kind)
		{
		case SpawnKind::PLAYER: 
			result.player = SpawnActor(scene, "Player", s.feet); result.player.AddComponent<Player>();
			break;
		case SpawnKind::ENEMY:  
			SpawnActor(scene, "Enemy", s.feet).AddComponent<Enemy>(); 
			break;
		case SpawnKind::COIN:   
			scene.CreateEntity("Coin").AddComponent<Pickup>(Pickup{ { s.feet.x, s.feet.y + kPickupHalf } }); 
			++result.coins; 
			break;
		case SpawnKind::SPIKE:  
			scene.CreateEntity("Spike").AddComponent<Hazard>(Hazard{ { { s.feet.x, s.feet.y + kSpikeHalf.y }, kSpikeHalf } }); 
			break;
		case SpawnKind::GOAL:   
			scene.CreateEntity("Goal").AddComponent<Goal>(Goal{ { { s.feet.x, s.feet.y + kGoalHalf.y }, kGoalHalf } }); 
			break;

		default:
			break;
		}
	}
	return result;
}
