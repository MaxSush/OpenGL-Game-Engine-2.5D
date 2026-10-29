#pragma once

#include "entity.h"

#include <entt/fwd.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace Engine
{
	namespace Detail
	{
		[[noreturn]] ENGINE_API void FailFast(const char* msg);	// log critical, abort
		ENGINE_API void ReportError(const char* msg);			// log error; assert in Debug
	}

	class ENGINE_API Scene
	{
	public:
		explicit Scene(std::string name);
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene&&) = delete;

		Entity CreateEntity(std::string_view name = {});
		void DestroyEntity(Entity entity);	// storage released at the next Flush
		bool IsValid(Entity entity) const;
		void Clear();	// destroys everything now

		template<typename ... Ts, typename Func>
		void Each(Func&& fn);
		
		void Defer(std::function<void(Scene&)> command);
		void Flush();	// release destroyed entities

		std::string_view Name() const;
		size_t EntityCount() const;
		bool IsIterating() const;

	private:
		friend class Entity;
		friend class World;
		struct Impl;

		entt::registry& GetRegistry() const;
		bool Resolve(Entity entity, entt::entity& out) const;
		bool MakeHandle(entt::entity id, Entity& out) const;
		void BeginIteration();
		void EndIteration();
		void AssertStructuralChangeAllowed(const char* what) const;
		void ReclaimPending();
		static Scene* Find(uint32_t scene_id);

		uint32_t m_scene_id = 0;
		std::unique_ptr<Impl> m_impl;
	};
}

#include "scene_inl.h"