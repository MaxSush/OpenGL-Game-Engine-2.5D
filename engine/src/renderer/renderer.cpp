#include <engine/renderer/renderer.h>

#include <glm/gtc/type_ptr.hpp>
#include <engine/core/logger/log.h>

#include "buffer.h"
#include "shader.h"

#include <cstddef>

namespace Engine
{
	namespace
	{
		constexpr const char* kVertexSrc = R"(
			#version 330 core
			layout(location = 0) in vec2 a_position;
			layout(location = 1) in vec4 a_color;
			uniform mat4 u_view_proj;
			out vec4 v_color;
			void main()
			{
				v_color = a_color;
				gl_Position = u_view_proj * vec4(a_position, 0.0, 1.0);
			}
		)";
		constexpr const char* kFragmentSrc = R"(
			#version 330 core
			in vec4 v_color;
			out vec4 o_color;
			void main() 
			{ 
				o_color = v_color;
			}
		)";
	}

	struct Renderer::Impl
	{
		static constexpr uint32_t MaxQuads = 10000;
		static constexpr uint32_t MaxVertices = MaxQuads * 4;
		static constexpr uint32_t MaxIndices = MaxQuads * 6;

		struct Vertex
		{
			glm::vec2 position;
			glm::vec4 color;
		};
		std::vector<Vertex> vertices;

		GL::Shader shader{ kVertexSrc, kFragmentSrc };
		GL::VertexArray vao;
		GL::BufferObject vbo{ GL_ARRAY_BUFFER };
		GL::BufferObject ibo{ GL_ELEMENT_ARRAY_BUFFER };

		GLint u_view_proj = -1;
		uint32_t quad_count = 0;
		glm::mat4 view_proj{ 1.0f };

		RendererStats stats{};
		bool in_scene = false;
		bool warned_outside_scene = false;

		Impl()
			: vertices(MaxVertices)
		{
			u_view_proj = shader.Uniform("u_view_proj");
			vao.Bind();
			vbo.Bind();

			glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(MaxVertices * sizeof(Vertex)), nullptr, GL_DYNAMIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, position)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, color)));

			std::vector<uint32_t> indices(MaxIndices);
			for (uint32_t i = 0, v = 0; i < MaxIndices; i += 6, v += 4)
			{
				indices[i + 0] = v + 0;
				indices[i + 1] = v + 1;
				indices[i + 2] = v + 2;
				indices[i + 3] = v + 2;
				indices[i + 4] = v + 3;
				indices[i + 5] = v + 0;
			}
			ibo.Bind();
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)), indices.data(), GL_STATIC_DRAW);
			vao.Unbind();
		}

		void Flush()
		{
			if (quad_count == 0)
			{
				return;
			}

			shader.Use();
			glUniformMatrix4fv(u_view_proj, 1, GL_FALSE, glm::value_ptr(view_proj));
			vao.Bind();
			vbo.Bind();
			glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(quad_count * 4 * sizeof(Vertex)), vertices.data());
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(quad_count * 6), GL_UNSIGNED_INT, nullptr);

			++stats.draw_calls;
			quad_count = 0;
		}
	};

	Renderer::Renderer()
		: m_impl(std::make_unique<Impl>())
	{
		if (GLenum err = glGetError(); err != GL_NO_ERROR)
		{
			LOG_ERROR("Renderer2D init left GL error 0x{:X}", err);
		}
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::BeginScene(const Camera& camera)
	{
		if (m_impl->in_scene)
		{
			LOG_WARN("Renderer2D::BeginScene called twice without EndScene");
		}
		m_impl->view_proj = camera.ViewProjection();
		m_impl->quad_count = 0;
		m_impl->in_scene = true;
	}

	void Renderer::DrawQuad(const glm::vec2& center, const glm::vec2& size, const glm::vec4& color)
	{
		if (!m_impl->in_scene)
		{
			if (!m_impl->warned_outside_scene)
			{
				LOG_WARN("Renderer2D::DrawQuad called outside BeginScene/EndScene (ignored)");
				m_impl->warned_outside_scene = true;
			}
			return;
		}

		if (m_impl->quad_count == Impl::MaxQuads)
		{
			m_impl->Flush();
		}

		const glm::vec2 h = size * 0.5f;
		Impl::Vertex* v = &m_impl->vertices[m_impl->quad_count * 4];
		v[0] = { {center.x - h.x, center.y - h.y}, color }; // bottom-left
		v[1] = { {center.x + h.x, center.y - h.y}, color }; // bottom-right
		v[2] = { {center.x + h.x, center.y + h.y}, color }; // top-right
		v[3] = { {center.x - h.x, center.y + h.y}, color }; // top-left
		++m_impl->quad_count;
		++m_impl->stats.quad_count;
	}

	void Renderer::EndScene()
	{
		m_impl->Flush();
		m_impl->in_scene = false;
	}

	const RendererStats& Renderer::GetStats() const
	{
		return m_impl->stats;
	}

	void Renderer::BeginFrame(const std::array<float, 4>& c)
	{
		m_impl->stats = {};

		if (m_impl->in_scene)
		{
			LOG_WARN("Renderer2D: previous scene was not ended (missing EndScene)");
			m_impl->in_scene = false;
			m_impl->quad_count = 0;
		}

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(c[0], c[1], c[2], c[3]);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Renderer::SetViewport(int width, int height)
	{
		if (width <= 0 || height <= 0)
		{
			return;
		}
		glViewport(0, 0, width, height);
	}
}