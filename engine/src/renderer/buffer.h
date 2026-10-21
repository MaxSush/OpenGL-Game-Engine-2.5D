#pragma once

#include <glad/gl.h>

namespace Engine::GL
{
	class BufferObject
	{
	public:
		explicit BufferObject(GLenum target)
			: m_target(target)
		{
			glGenBuffers(1, &m_id);
		}

		~BufferObject()
		{
			if (m_id)
			{
				glDeleteBuffers(1, &m_id);
			}
		}

		BufferObject(const BufferObject&) = delete;
		BufferObject& operator=(const BufferObject&) = delete;

		void Bind() const
		{
			glBindBuffer(m_target, m_id);
		}

	private:
		GLenum m_target;
		GLuint m_id = 0;
	};


	class VertexArray
	{
	public:
		VertexArray()
		{
			glGenVertexArrays(1, &m_id);
		}

		~VertexArray()
		{
			if (m_id)
			{
				glDeleteVertexArrays(1, &m_id);
			}
		}

		VertexArray(const VertexArray&) = delete;
		VertexArray& operator=(const VertexArray&) = delete;

		void Bind() const
		{
			glBindVertexArray(m_id);
		}

		void Unbind() const
		{
			glBindVertexArray(0);
		}
	private:
		GLuint m_id = 0;
	};

}