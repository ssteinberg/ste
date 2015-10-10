// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "GLSLShader.h"

namespace StE {
namespace LLR {

class GLSLProgram {
private:
	GLuint  id;
	bool linked;
	std::vector<std::unique_ptr<GLSLShader>> shaders;

	int  get_uniform_location(const char * name) const;

public:
	GLSLProgram(GLSLProgram &&m) = default;
	GLSLProgram(const GLSLProgram &c) = delete;
	GLSLProgram& operator=(GLSLProgram &&m) = default;
	GLSLProgram& operator=(const GLSLProgram &c) = delete;

	GLSLProgram() : linked(false) { id = glCreateProgram(); }
	virtual ~GLSLProgram() { if (is_valid()) glDeleteProgram(id); }

	void	add_shader(std::unique_ptr<GLSLShader> shader) { 
		if (shader == nullptr || !shader->is_valid()) {
			assert(false);
			return;
		}
		shaders.push_back(std::move(shader));
		linked = false; 
	}

	bool	link();
	bool	is_linked() { return linked; }

	bool	link_from_binary(unsigned format, const std::string &data);
	std::string get_binary_represantation(unsigned *format);

	void	bind() const {
		if (id <= 0 || (!linked)) {
			assert(false);
			return;
		}
		glUseProgram(id);
	}

	void	bind_attrib_location(GLuint location, const char * name) { glBindAttribLocation(id, location, name); }
	void	bind_frag_data_location(GLuint location, const char * name) { glBindFragDataLocation(id, location, name); }

	void	set_uniform(const char *name, float x, float y, float z) const;
	void	set_uniform(const char *name, const glm::vec2 & v) const;
	void	set_uniform(const char *name, const glm::i32vec2 & v) const;
	void	set_uniform(const char *name, const glm::vec3 & v) const;
	void	set_uniform(const char *name, const glm::vec4 & v) const;
	void	set_uniform(const char *name, const glm::mat2 & m) const;
	void	set_uniform(const char *name, const glm::mat3 & m) const;
	void	set_uniform(const char *name, const glm::mat4 & m) const;
	void	set_uniform(const char *name, float val) const;
	void	set_uniform(const char *name, int val) const;
	void	set_uniform(const char *name, bool val) const;

	void	print_active_uniforms();
	void	print_active_attribs();

	bool	is_valid() const { return id != 0; }

	GLuint	get_program_id() { return id; }
};

}
}
