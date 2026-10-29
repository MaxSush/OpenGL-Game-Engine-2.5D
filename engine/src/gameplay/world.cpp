#include <engine/gameplay/world.h>

namespace Engine
{
	struct World::Impl
	{
		std::vector<std::unique_ptr<Scene>> scenes;
		Scene* active = nullptr;
	};

	World::World()
		: m_impl(std::make_unique<Impl>())
	{
	}

	World::~World()
	{
		Clear();
	}

	Scene& World::CreateScene(std::string_view name)
	{
		if (name.empty())
		{
			Detail::FailFast("World::CreateScene => empty scene name");
		}
		if (FindScene(name))
		{
			Detail::FailFast("World::CreateScene => duplicate scene name");
		}
		m_impl->scenes.push_back(std::make_unique<Scene>(std::string(name)));
		return *m_impl->scenes.back();
	}

	Scene* World::FindScene(std::string_view name)
	{
		for (auto& scene : m_impl->scenes)
		{
			if (scene->Name() == name)
			{
				return scene.get();
			}
		}
		return nullptr;
	}

	bool World::SetActiveScene(std::string_view name)
	{
		Scene* scene = FindScene(name);
		if (!scene)
		{
			return false;
		}
		m_impl->active = scene;
		return true;
	}

	bool World::HasActiveScene() const
	{
		return (m_impl->active != nullptr);
	}

	Scene& World::GetActiveScene()
	{
		if (!m_impl->active)
		{
			Detail::FailFast("World::GetActiveScene => no active scene");
		}
		return *m_impl->active;
	}

	bool World::DestroyScene(std::string_view name)
	{
		auto& scenes = m_impl->scenes;
		const auto it = std::find_if(scenes.begin(), scenes.end(),
			[&](const auto& scene)
			{
				return scene->Name() == name;
			}
		);
		if (it == scenes.end())
		{
			return false;
		}
		if ((*it)->IsIterating())
		{
			Detail::ReportError("World::DestroyScene => call while the scene is iterating"); 
			return false;
		}

		if (m_impl->active == it->get())
		{
			m_impl->active = nullptr;
		}
		scenes.erase(it);
		return true;
	}

	void World::Flush()
	{
		for (auto& scene : m_impl->scenes)
		{
			scene->Flush();
		}
	}

	void World::Clear()
	{
		m_impl->active = nullptr;
		m_impl->scenes.clear();
	}
}