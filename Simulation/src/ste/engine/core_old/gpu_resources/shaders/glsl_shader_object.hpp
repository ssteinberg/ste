// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <string>
#include <glm/glm.hpp>

#include <resource_old.hpp>

namespace StE {
namespace Core {

enum class glsl_shader_type {
	NONE,
	VERTEX, FRAGMENT, GEOMETRY,
	COMPUTE,
	TESS_CONTROL, TESS_EVALUATION
};

struct glsl_shader_properties {
	int version_major, version_minor;
};

namespace _ste_glslshader_creator {
	template <glsl_shader_type T>
	generic_resource::type inline creator() { assert(false && "unspecialized"); return 0; }
	template <> generic_resource::type inline creator<glsl_shader_type::NONE>() { assert(false && "ShaderType cannot be none."); return 0; }
	template <> generic_resource::type inline creator<glsl_shader_type::VERTEX>() { return glCreateShader(GL_VERTEX_SHADER); }
	template <> generic_resource::type inline creator<glsl_shader_type::FRAGMENT>() { return glCreateShader(GL_FRAGMENT_SHADER); }
	template <> generic_resource::type inline creator<glsl_shader_type::GEOMETRY>() { return glCreateShader(GL_GEOMETRY_SHADER); }
	template <> generic_resource::type inline creator<glsl_shader_type::COMPUTE>() { return glCreateShader(GL_COMPUTE_SHADER); }
	template <> generic_resource::type inline creator<glsl_shader_type::TESS_CONTROL>() { return glCreateShader(GL_TESS_CONTROL_SHADER); }
	template <> generic_resource::type inline creator<glsl_shader_type::TESS_EVALUATION>() { return glCreateShader(GL_TESS_EVALUATION_SHADER); }
};

template <glsl_shader_type ShaderType>
class glsl_shader_allocator : public generic_resource_allocator {
public:
	generic_resource::type allocate() override final {
		generic_resource::type res = _ste_glslshader_creator::creator<ShaderType>();
		return res;
	}
	static void deallocate(generic_resource::type &id) {
		if (id) {
			glDeleteShader(id);
			id = 0;
		}
	}
};

class glsl_shader_object_generic : virtual public generic_resource {
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

template <glsl_shader_type ShaderType>
class glsl_shader_object : public resource<glsl_shader_allocator<ShaderType>>, public glsl_shader_object_generic {
private:
	int status;
	glsl_shader_properties properties;

	void compile() {
		glCompileShader(get_resource_id());
		glGetShaderiv(get_resource_id(), GL_COMPILE_STATUS, &status);
	}

public:
	static constexpr glsl_shader_type type = ShaderType;

public:
	glsl_shader_object(glsl_shader_object &&m) = default;
	glsl_shader_object(const glsl_shader_object &c) = delete;
	glsl_shader_object& operator=(glsl_shader_object &&m) = default;
	glsl_shader_object& operator=(const glsl_shader_object &c) = delete;

	glsl_shader_object(const std::string &name, const std::string &src, const glsl_shader_properties &properties) : glsl_shader_object_generic(name), properties(properties) {
		auto str = src.data();
		glShaderSource(get_resource_id(), 1, &str, nullptr);

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
	const glsl_shader_properties &get_shader_properties() const { return properties; }

	core_resource_type resource_type() const override { return core_resource_type::GLSLShader; }
};

}
}
