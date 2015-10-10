// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <string>

#include <glm/glm.hpp>

namespace StE {
namespace LLR {

class GLSLShader {
public:
	enum class GLSLShaderType {
		NONE = 0,
		VERTEX, FRAGMENT, GEOMETRY, 
		COMPUTE,
		TESS_CONTROL, TESS_EVALUATION
	};
	
private:
	GLSLShaderType type{ GLSLShaderType::NONE };
	GLuint id;

public:
	GLSLShader(GLSLShader &&m) = default;
	GLSLShader(const GLSLShader &c) = delete;
	GLSLShader& operator=(GLSLShader &&m) = default;
	GLSLShader& operator=(const GLSLShader &c) = delete;

	GLSLShader(GLSLShaderType type) : type(type) {
		switch (type) {
		case GLSLShaderType::VERTEX:			id = glCreateShader(GL_VERTEX_SHADER); break;
		case GLSLShaderType::FRAGMENT:			id = glCreateShader(GL_FRAGMENT_SHADER); break;
		case GLSLShaderType::GEOMETRY:			id = glCreateShader(GL_GEOMETRY_SHADER); break;
		case GLSLShaderType::COMPUTE:			id = glCreateShader(GL_COMPUTE_SHADER); break;
		case GLSLShaderType::TESS_CONTROL:		id = glCreateShader(GL_TESS_CONTROL_SHADER); break;
		case GLSLShaderType::TESS_EVALUATION:	id = glCreateShader(GL_TESS_EVALUATION_SHADER); break;
		default:								assert(false); id = 0; break;
		}
	}
	virtual ~GLSLShader() { if (this->is_valid()) glDeleteShader(id); }

	void set_shader_source(const std::string &src) { assert(is_valid); auto str = src.data(); glShaderSource(id, 1, &str, NULL); }
	void attach_to_program(GLuint program) const { assert(is_valid); glAttachShader(program, id); }

	bool compile() { 
		assert(is_valid);
		glCompileShader(id);
		GLint result;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);
		return result != false;
	}

	std::string read_info_log() {
		GLint length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			std::unique_ptr<char> log(new char[length]);
			int written = 0;
			glGetShaderInfoLog(id, length, &written, &*log);
			return &*log;
		}
		return std::string();
	}

	bool is_valid() const { return id != 0; }

protected:
	GLuint get_shader_id() const { return id; }
};

}
}
