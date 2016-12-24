// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include <string>
#include <glm/glm.hpp>

#include "resource.hpp"

namespace StE {
namespace Core {

enum class GLSLShaderType {
	NONE,
	VERTEX, FRAGMENT, GEOMETRY,
	COMPUTE,
	TESS_CONTROL, TESS_EVALUATION
};

struct GLSLShaderProperties {
	int version_major, version_minor;
};

namespace _ste_glslshader_creator {
	template <GLSLShaderType T>
	GenericResource::type inline creator() { assert(false && "unspecialized"); return 0; }
	template <> GenericResource::type inline creator<GLSLShaderType::NONE>() { assert(false && "ShaderType cannot be none."); return 0; }
	template <> GenericResource::type inline creator<GLSLShaderType::VERTEX>() { return glCreateShader(GL_VERTEX_SHADER); }
	template <> GenericResource::type inline creator<GLSLShaderType::FRAGMENT>() { return glCreateShader(GL_FRAGMENT_SHADER); }
	template <> GenericResource::type inline creator<GLSLShaderType::GEOMETRY>() { return glCreateShader(GL_GEOMETRY_SHADER); }
	template <> GenericResource::type inline creator<GLSLShaderType::COMPUTE>() { return glCreateShader(GL_COMPUTE_SHADER); }
	template <> GenericResource::type inline creator<GLSLShaderType::TESS_CONTROL>() { return glCreateShader(GL_TESS_CONTROL_SHADER); }
	template <> GenericResource::type inline creator<GLSLShaderType::TESS_EVALUATION>() { return glCreateShader(GL_TESS_EVALUATION_SHADER); }
};

template <GLSLShaderType ShaderType>
class GLSLShaderAllocator : public generic_resource_allocator {
public:
	GenericResource::type allocate() override final {
		GenericResource::type res = _ste_glslshader_creator::creator<ShaderType>();
		return res;
	}
	static void deallocate(GenericResource::type &id) {
		if (id) {
			glDeleteShader(id);
			id = 0;
		}
	}
};

class glsl_shader_object_generic : virtual public GenericResource {
private:
	std::string name;

protected:
	glsl_shader_object_generic(const std::string &name) : name(name) {}

public:
	glsl_shader_object_generic(glsl_shader_object_generic &&m) = default;
	glsl_shader_object_generic(const glsl_shader_object_generic &c) = default;
	glsl_shader_object_generic& operator=(glsl_shader_object_generic &&m) = default;
	glsl_shader_object_generic& operator=(const glsl_shader_object_generic &c) = default;

	auto &get_name() const { return name; }

	virtual std::string read_info_log() const = 0;
	virtual bool get_status() const = 0;

	virtual ~glsl_shader_object_generic() noexcept {}
};

template <GLSLShaderType ShaderType>
class glsl_shader_object : public resource<GLSLShaderAllocator<ShaderType>>, public glsl_shader_object_generic {
private:
	int status;
	GLSLShaderProperties properties;

	void compile() {
		glCompileShader(get_resource_id());
		glGetShaderiv(get_resource_id(), GL_COMPILE_STATUS, &status);
	}

public:
	static constexpr GLSLShaderType type = ShaderType;

public:
	glsl_shader_object(glsl_shader_object &&m) = default;
	glsl_shader_object(const glsl_shader_object &c) = delete;
	glsl_shader_object& operator=(glsl_shader_object &&m) = default;
	glsl_shader_object& operator=(const glsl_shader_object &c) = delete;

	glsl_shader_object(const std::string &name, const std::string &src, const GLSLShaderProperties &properties) : glsl_shader_object_generic(name), properties(properties) {
		auto str = src.data();
		glShaderSource(get_resource_id(), 1, &str, NULL);

		compile();
	}

	std::string read_info_log() const override final {
		GLint length;
		glGetShaderiv(get_resource_id(), GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			std::string log;
			log.resize(length-1);		// Ignore the '\n' at the end...
			int written = 0;
			glGetShaderInfoLog(get_resource_id(), length-1, &written, &log[0]);
			return log;
		}
		return std::string();
	}

	bool get_status() const override final { return !!status; }
	const GLSLShaderProperties &get_shader_properties() const { return properties; }

	core_resource_type resource_type() const override { return core_resource_type::GLSLShader; }
};

}
}
