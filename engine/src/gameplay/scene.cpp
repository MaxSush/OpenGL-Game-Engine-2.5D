#include <engine/gameplay/scene.h>

#include <engine/gameplay/component.h>

#include <engine/core/logger/log.h>

namespace Engine
{
	namespace
	{
		struct EntitySlot
		{
			uint32_t generation = 1;
			bool alive = false;
			entt::entity entt_handle = entt::null;
		};

		// Current active scenes present
		std::vector<Scene*>& LiveScenes()
		{
			static std::vector<Scene*> scenes;
			return scenes;
		}

		// global counter for unique scene id 
		uint32_t g_next_scene_id = 1;
	}

	namespace Detail
	{
		void FailFast(const char* msg)
		{
			LOG_CRITICAL("{}", msg);
			std::abort();
		}

		void ReportError(const char* msg)
		{
			LOG_ERROR("{}", msg);
			assert(false && "gameplay error, see the log");
		}
	}

	struct Scene::Impl
	{
		std::string name;
		entt::registry registry;
		std::vector<EntitySlot> slots;
		std::vector<entt::entity> pending_destroy;
		std::vector<std::function<void(Scene&)>> commands;
		size_t alive = 0;
		int iterating = 0;
	};

	Scene::Scene(std::string name)
		: m_scene_id(g_next_scene_id++),
		m_impl(std::make_unique<Impl>())
	{
		m_impl->name = std::move(name);
		LiveScenes().push_back(this);
	}

	Scene::~Scene()
	{
		// remove current scene from global vector scenes
		auto& live = LiveScenes();
		live.erase(std::remove(live.begin(), live.end(), this), live.end());
	}

	Scene* Scene::Find(uint32_t scene_id)
	{
		if (scene_id == 0)
		{
			return nullptr;
		}
		for (Scene* scene : LiveScenes())
		{
			if (scene->m_scene_id == scene_id)
			{
				return scene;
			}
		}
		return nullptr;
	}

	entt::registry& Scene::GetRegistry() const
	{
		return m_impl->registry;
	}

	Entity Scene::CreateEntity(std::string_view name)
	{
		AssertStructuralChangeAllowed("Scene::CreateEntity");
		
		const entt::entity id = m_impl->registry.create();
		const auto entity_id = static_cast<uint32_t>(entt::to_entity(id));
		if (entity_id >= m_impl->slots.size())
		{
			m_impl->slots.resize(static_cast<size_t>(entity_id) + 1);
		}

		EntitySlot& slot = m_impl->slots[entity_id];
		slot.alive = true;
		slot.entt_handle = id;
		m_impl->alive++;

		Entity e = Entity(m_scene_id, entity_id, slot.generation);
		if (name.empty())
		{
			name = "Entity_" + std::to_string(e.m_entity_id);
		}
		e.AddComponent<TagNameComponent>(std::string(name));
		return e;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (entity.m_scene_id != m_scene_id)
		{
			if (entity.m_scene_id != 0)
			{
				Detail::ReportError("Scene::DestroyEntity => entity belongs to another scene");
				return;
			}
		}

		entt::entity id{};
		if (!Resolve(entity, id))
		{
			return;
		}

		EntitySlot& slot = m_impl->slots[entity.m_entity_id];
		slot.alive = false;
		++slot.generation;
		m_impl->alive--;
		m_impl->pending_destroy.push_back(id);
	}

	bool Scene::IsValid(Entity entity) const
	{
		entt::entity id{};
		return Resolve(entity, id);
	}

	void Scene::Clear()
	{
		assert(m_impl->iterating == 0 && "Scene::Clear inside Each: use Scene::Defer");
		
		m_impl->registry.clear();
		for (EntitySlot& slot : m_impl->slots)
		{
			slot.alive = false;
			++slot.generation;
			slot.entt_handle = entt::null;
		}
		m_impl->pending_destroy.clear();
		m_impl->commands.clear();
		m_impl->alive = 0;
	}

	void Scene::Defer(std::function<void(Scene&)> command)
	{
		m_impl->commands.push_back(std::move(command));
	}

	void Scene::Flush()
	{
		assert(m_impl->iterating == 0 && "Scene::Flush inside Each");

		ReclaimPending();
		if (!m_impl->commands.empty())
		{
			auto running = std::exchange(m_impl->commands, {});		// commands queued by commands wait for the next Flush
			for (auto& command : running)
			{
				command(*this);
			}
			ReclaimPending();
		}
	}

	bool Scene::Resolve(Entity entity, entt::entity& out) const
	{
		if (entity.m_scene_id != m_scene_id)
		{
			return false;
		}

		const auto& slots = m_impl->slots;
		if (entity.m_entity_id >= slots.size())
		{
			return false;
		}

		const EntitySlot& slot = slots[entity.m_entity_id];
		if (!slot.alive || slot.generation != entity.m_generation_id)
		{
			return false;
		}
		out = slot.entt_handle;
		return true;
	}

	bool Scene::MakeHandle(entt::entity handle, Entity& out) const
	{
		const auto entity_id = static_cast<uint32_t>(entt::to_entity(handle));
		const auto& slots = m_impl->slots;
		if (entity_id >= slots.size())
		{
			return false;
		}
		const EntitySlot& slot = slots[entity_id];
		if (!slot.alive || slot.entt_handle != handle)
		{
			return false;
		}
		out = Entity(m_scene_id, entity_id, slot.generation);
		return true;
	}
	
	void Scene::ReclaimPending()
	{
		for (const entt::entity handle : m_impl->pending_destroy)
		{
			if (m_impl->registry.valid(handle))
			{
				m_impl->registry.destroy(handle);	// removes its components, then releases the id
			}
		}
		m_impl->pending_destroy.clear();
	}

	std::string_view Scene::Name() const
	{
		return m_impl->name;
	}

	bool Scene::IsIterating() const
	{
		return m_impl->iterating > 0;
	}

	void Scene::BeginIteration()
	{
		++m_impl->iterating;
	}

	void Scene::EndIteration()
	{
		--m_impl->iterating;
	}

	void Scene::AssertStructuralChangeAllowed(const char* what) const
	{
		if (IsIterating())
		{
			LOG_ERROR("{} inside Scene::Each: use Scene::Defer", what);
			assert(false && "structural change during iteration");
		}
	}

	// Entity class methods

	bool Entity::IsValid() const
	{
		const Scene* scene = Scene::Find(m_scene_id);
		return scene && scene->IsValid(*this);
	}

	size_t Scene::EntityCount() const
	{
		return m_impl->alive;
	}

	void Entity::Destroy() const
	{
		if (Scene* scene = Scene::Find(m_scene_id))
		{
			scene->DestroyEntity(*this);
		}
	}
}