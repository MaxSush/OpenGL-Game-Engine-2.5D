#pragma once

#include <cstdint>
#include <string_view>

#include <engine/engine_api.h>

namespace Engine
{
	class Scene;

	class ENGINE_API Entity
	{
	public:
		Entity() = default;

		bool IsValid() const;
		explicit operator bool() const
		{
			return IsValid();
		}
		bool operator==(const Entity&) const = default;

		template<typename T, typename ... Args>
		T& AddComponent(Args&& ... args) const;

		template<typename T, typename ... Args>
		T& AddOrReplaceComponent(Args&& ... args) const;

		template<typename T>
		T& GetComponent() const;

		template<typename T>
		T* TryGetComponent() const;

		template<typename T>
		bool HasComponent() const;

		template<typename T>
		bool RemoveComponent();

		void Destroy() const;

	private:
		friend class Scene;

		Entity(uint32_t scene, uint32_t entity, uint32_t generation) noexcept
			: m_scene_id(scene),
			m_entity_id(entity),
			m_generation_id(generation)
		{
		}

		uint32_t m_scene_id = 0;		// scene id to assign to
		uint32_t m_entity_id = 0;		// entity id cast to uint32
		uint32_t m_generation_id = 0;	// entity id can be reused after destroyed, this identifies current uniquely from destroyed
	};
}