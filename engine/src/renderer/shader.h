#pragma once

#include <glad/gl.h>

#include <stdexcept>
#include <string>

namespace Engine::GL
{
	inline GLuint CompileShader(GLenum type, const char* src)
	{
		GLuint s = glCreateShader(type);
		glShaderSource(s, 1, &src, nullptr);
		glCompileShader(s);

		GLint ok = 0;
		glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
		if (!ok)
		{
			char log[1024];
			glGetShaderInfoLog(s, sizeof(log), nullptr, log);
			glDeleteShader(s);
			throw std::runtime_error(std::string("Shader compile failed: ") + log);
		}
		return s;
	}

	class Shader
	{
	public:
		Shader(const char* vertex_shader, const char* fragment_shader)
		{
			GLuint vs = CompileShader(GL_VERTEX_SHADER, vertex_shader);
			GLuint fs = 0;
			try
			{
				fs = CompileShader(GL_FRAGMENT_SHADER, fragment_shader);
			}
			catch (...)
			{
				glDeleteShader(vs);
				throw;
			}

			m_id = glCreateProgram();
			glAttachShader(m_id, vs);
			glAttachShader(m_id, fs);
			glLinkProgram(m_id);

			glDeleteShader(vs);
			glDeleteShader(fs);

			GLint ok = 0;
			glGetProgramiv(m_id, GL_LINK_STATUS, &ok);
			if (!ok)
			{
				char log[1024];
				glGetProgramInfoLog(m_id, sizeof(log), nullptr, log);
				glDeleteProgram(m_id);
				m_id = 0;
				throw std::runtime_error(std::string("Shader Link Failed: ") + log);
			}
		}

		~Shader()
		{
			if (m_id)
			{
				glDeleteProgram(m_id);
			}
		}

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		void Use() const
		{
			glUseProgram(m_id);
		}

		GLint Uniform(const char* name) const
		{
			return glGetUniformLocation(m_id, name);
		}
	private:
		GLuint m_id = 0;
	};
}