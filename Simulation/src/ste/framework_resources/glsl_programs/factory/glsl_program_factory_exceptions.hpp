// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <resource_loading_exception.hpp>

namespace StE {
namespace Resource {

class glsl_program_error : public resource_loading_exception {
public:
	using resource_loading_exception::resource_loading_exception;
};

class glsl_program_undefined_program_error : public glsl_program_error {
public:
	using glsl_program_error::glsl_program_error;
};

class glsl_program_undefined_shader_error : public glsl_program_error {
public:
	using glsl_program_error::glsl_program_error;
};

class glsl_program_shader_compilation_error : public glsl_program_error {
public:
	using glsl_program_error::glsl_program_error;
};

class glsl_program_linking_error : public glsl_program_error {
public:
	using glsl_program_error::glsl_program_error;
};

}
}
