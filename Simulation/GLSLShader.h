// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <string>
#include <glm/glm.hpp>

#include "resource.h"

namespace StE {
namespace LLR {

enum class GLSLShaderType {
	VERTEX, FRAGMENT, GEOMETRY,
	COMPUTE,
	TESS_CONTROL, TESS_EVALUATION
};

struct GLSLShaderProperties {
	int version_major, version_minor;
};

template <GLSLShaderType ShaderType>
class GLSLShaderAllocator : public generic_resource_allocator {
public:
	unsigned allocate() override final {
		switch (ShaderType) {
		case GLSLShaderType::VERTEX:			return glCreateShader(GL_VERTEX_SHADER);
		case GLSLShaderType::FRAGMENT:			return glCreateShader(GL_FRAGMENT_SHADER);
		case GLSLShaderType::GEOMETRY:			return glCreateShader(GL_GEOMETRY_SHADER);
		case GLSLShaderType::COMPUTE:			return glCreateShader(GL_COMPUTE_SHADER);
		case GLSLShaderType::TESS_CONTROL:		return glCreateShader(GL_TESS_CONTROL_SHADER);
		case GLSLShaderType::TESS_EVALUATION:	return glCreateShader(GL_TESS_EVALUATION_SHADER);
		default:								assert(false); return 0;
		}
	}
	static void deallocate(unsigned &id) { glDeleteShader(id); id = 0; }
};

class GLSLShaderGeneric : virtual public GenericResource {
public:
	virtual std::string read_info_log() const = 0;
	virtual bool get_status() const = 0;

	virtual ~GLSLShaderGeneric() noexcept {}
};

template <GLSLShaderType ShaderType>
class GLSLShader : public resource<GLSLShaderAllocator<ShaderType>>, public GLSLShaderGeneric {
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
	GLSLShader(GLSLShader &&m) = default;
	GLSLShader(const GLSLShader &c) = delete;
	GLSLShader& operator=(GLSLShader &&m) = default;
	GLSLShader& operator=(const GLSLShader &c) = delete;

	GLSLShader(const std::string &src, const GLSLShaderProperties &properties) : properties(properties) {
		auto str = src.data();
		glShaderSource(get_resource_id(), 1, &str, NULL);

		compile();
	}

	std::string read_info_log() const override final {
		GLint length;
		glGetShaderiv(get_resource_id(), GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			std::string log;
			log.resize(length);
			int written = 0;
			glGetShaderInfoLog(get_resource_id(), length, &written, &log[0]);
			return log;
		}
		return std::string();
	}

	bool get_status() const override final { return !!status; }
	const GLSLShaderProperties &get_shader_properties() const { return properties; }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRGLSLShader; }
};

}
}
