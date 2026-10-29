#pragma once

#include <engine/gameplay/scene.h>

#include <string_view>
#include <memory>

namespace Engine
{
	class ENGINE_API World
	{
	public:
		World();
		~World();

		World(const World&) = delete;
		World& operator=(const World&) = delete;

		Scene& CreateScene(std::string_view name);
		Scene* FindScene(std::string_view name);
		
		bool SetActiveScene(std::string_view name);
		bool HasActiveScene() const;
		Scene& GetActiveScene();

		bool DestroyScene(std::string_view name);
		void Flush();
		void Clear();

	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};
}