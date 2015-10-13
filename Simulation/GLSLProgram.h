// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "bindable_resource.h"
#include "GLSLShader.h"

namespace StE {
namespace LLR {

class GLSLProgramAllocator : public llr_resource_stub_allocator {
public:
	static int allocate() { return glCreateProgram(); }
	static void deallocate(unsigned int &id) { glDeleteProgram(id); id = 0; }
};

class GLSLProgramBinder {
public:
	static void bind(unsigned int id) { glUseProgram(id); }
	static void unbind() { glUseProgram(0); }
};

class GLSLProgram : public bindable_resource<GLSLProgramAllocator, GLSLProgramBinder> {
private:
	bool linked;
	std::vector<std::unique_ptr<GLSLShaderGeneric>> shaders;
	mutable std::unordered_map<std::string, int> uniform_map;

	int get_uniform_location(const std::string &name) const {
		auto it = uniform_map.find(name);
		if (it != uniform_map.end())
			return it->second;

		int loc = glGetUniformLocation(get_resource_id(), name.c_str());
		if (loc >= 0)
			uniform_map[name] = loc;
		return loc;
	}

public:
	GLSLProgram(GLSLProgram &&m) = default;
	GLSLProgram(const GLSLProgram &c) = delete;
	GLSLProgram& operator=(GLSLProgram &&m) = default;
	GLSLProgram& operator=(const GLSLProgram &c) = delete;

	GLSLProgram() : linked(false) {}
	virtual ~GLSLProgram() {}

	void add_shader(std::unique_ptr<GLSLShaderGeneric> shader) {
		if (shader == nullptr || !shader->is_valid()) {
			assert(false);
			return;
		}
		shaders.push_back(std::move(shader));
		linked = false;
	}

	bool link();
	bool is_linked() const { return linked; }

	bool link_from_binary(unsigned format, const std::string &data);
	std::string get_binary_represantation(unsigned *format);

	void bind_attrib_location(GLuint location, const std::string &name) const { glBindAttribLocation(get_resource_id(), location, name.c_str()); }
	void bind_frag_data_location(GLuint location, const std::string &name) const { glBindFragDataLocation(get_resource_id(), location, name.c_str()); }

	void set_uniform(const std::string &name, float x, float y, float z) const;
	void set_uniform(const std::string &name, const glm::vec2 & v) const;
	void set_uniform(const std::string &name, const glm::i32vec2 & v) const;
	void set_uniform(const std::string &name, const glm::vec3 & v) const;
	void set_uniform(const std::string &name, const glm::vec4 & v) const;
	void set_uniform(const std::string &name, const glm::mat2 & m) const;
	void set_uniform(const std::string &name, const glm::mat3 & m) const;
	void set_uniform(const std::string &name, const glm::mat4 & m) const;
	void set_uniform(const std::string &name, float val) const;
	void set_uniform(const std::string &name, int val) const;
	void set_uniform(const std::string &name, bool val) const;

	llr_resource_type resource_type() const override { return llr_resource_type::LLRGLSLProgram; }
};

}
}
