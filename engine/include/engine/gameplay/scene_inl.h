#pragma once

#include <entt/entity/registry.hpp>

namespace Engine
{
	namespace Detail
	{
		template <typename T>
		constexpr void CheckComponent()
		{
			static_assert(std::is_same_v<T, std::remove_cv_t<std::remove_reference_t<T>>>, "component types must not be const or references");
			static_assert(!std::is_empty_v<T>, "==>(empty) components are not supported yet: give the component a field");
		}
	}

	template<typename T, typename ...Args>
	inline T& Entity::AddComponent(Args && ...args) const
	{
		Detail::CheckComponent<T>();

		Scene* scene = Scene::Find(m_scene_id);
		entt::entity id{};
		if (!scene || !scene->Resolve(*this, id))
		{
			Detail::FailFast("Entity::AddComponent => AddComponent on an invalid entity");
		}

		entt::registry& reg = scene->GetRegistry();
		if (reg.all_of<T>(id))
		{
			Detail::ReportError("Entity::AddComponent => component already present (use AddOrReplace)");
			return reg.get<T>(id);
		}

		scene->AssertStructuralChangeAllowed("Entity::Add");
		return reg.emplace<T>(id, std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	inline T& Entity::AddOrReplaceComponent(Args && ...args) const
	{
		Detail::CheckComponent<T>();

		Scene* scene = Scene::Find(m_scene_id);
		entt::entity id{};
		if (!scene || !scene->Resolve(*this, id))
		{
			Detail::FailFast("Entity::AddOrReplaceComponent => AddComponent on an invalid entity");
		}

		entt::registry& reg = scene->GetRegistry();
		if (reg.all_of<T>(id))
		{
			scene->AssertStructuralChangeAllowed("Entity::AddOrReplace");
			// replacing a value is fine mid-iteration
		}
		return reg.emplace_or_replace<T>(id, std::forward<Args>(args)...);
	}

	template<typename T>
	inline T& Entity::GetComponent() const
	{
		T* component = TryGetComponent<T>();
		if (!component)
		{
			Detail::FailFast("Entity::GetComponent => invalid entity or missing component");
		}
		return *component;
	}

	template<typename T>
	inline T* Entity::TryGetComponent() const
	{
		Detail::CheckComponent<T>();

		Scene* scene = Scene::Find(m_scene_id);
		entt::entity id{};
		if (!scene || !scene->Resolve(*this, id))
		{
			return nullptr;
		}
		return scene->GetRegistry().try_get<T>(id);
	}

	template<typename T>
	inline bool Entity::HasComponent() const
	{
		return TryGetComponent<T>() != nullptr;
	}

	template<typename T>
	inline bool Entity::RemoveComponent()
	{
		Detail::CheckComponent<T>();

		Scene* scene = Scene::Find(m_scene_id);
		entt::entity id{};
		if (!scene || !scene->Resolve(*this, id))
		{
			return false;
		}

		entt::registry& reg = scene->GetRegistry();
		if (!reg.all_of<T>(id))
		{
			return false;
		}
		scene->AssertStructuralChangeAllowed("Entity::RemoveComponent");
		reg.remove<T>(id);
		return true;
	}

	template<typename ... Ts, typename Func>
	void Scene::Each(Func&& fn)
	{
		static_assert(sizeof...(Ts) > 0, "Each needs at least one component type");
		(Detail::CheckComponent<Ts>(), ...);

		struct Scope
		{
			explicit Scope(Scene& s)
				: scene(s)
			{
				scene.BeginIteration();
			}

			~Scope()
			{
				scene.EndIteration();
			}

			Scene& scene;
		} scope(*this);

		auto view = GetRegistry().template view<Ts ...>();
		for (const entt::entity handle : view)
		{
			Entity entity;
			if (!MakeHandle(handle, entity))
			{
				continue;
			}
			fn(entity, view.template get<Ts>(handle)...);
		}
	}
}