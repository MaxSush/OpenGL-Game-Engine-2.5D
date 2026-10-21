#pragma once

#include <engine/engine_api.h>
#include <engine/renderer/camera.h>

#include <cstdint>
#include <memory>
#include <array>

namespace Engine
{
	class Application;

	struct RendererStats
	{
		uint32_t draw_calls = 0;
		uint32_t quad_count = 0;
	};

	class ENGINE_API Renderer
	{
	public:
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void BeginScene(const Camera& camera);
		void DrawQuad(const glm::vec2& center, const glm::vec2& size, const glm::vec4& color);
		void EndScene();

		const RendererStats& GetStats() const;

	private:
		friend class Application;
		Renderer();

		void BeginFrame(const std::array<float, 4>& clear_color);
		void SetViewport(int width, int height);

		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};
}