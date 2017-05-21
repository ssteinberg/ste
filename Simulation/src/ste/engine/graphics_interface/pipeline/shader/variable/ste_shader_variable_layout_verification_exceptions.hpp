//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vk_result.hpp>

#include <ste_engine_exceptions.hpp>

namespace ste {
namespace gl {

class ste_shader_variable_layout_verification_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

private:
	lib::string m;

public:
	ste_shader_variable_layout_verification_exception(const lib::string &m) : Base(m), m(m) {}
	const char* what() const override final { return m.c_str(); }

	void append(const lib::string &msg) {
		m += lib::string(" ") + msg;
	}
	void prepend(const lib::string &msg) {
		m = msg + " " + m;
	}
};

class ste_shader_variable_layout_verification_block_layout_mismatch : public ste_shader_variable_layout_verification_exception {
	using Base = ste_shader_variable_layout_verification_exception;

public:
	using Base::Base;
};

class ste_shader_variable_layout_verification_opaque_or_unknown_type : public ste_shader_variable_layout_verification_exception {
	using Base = ste_shader_variable_layout_verification_exception;

public:
	using Base::Base;
};

class ste_shader_variable_layout_verification_member_count_mismatch : public ste_shader_variable_layout_verification_exception {
	using Base = ste_shader_variable_layout_verification_exception;

public:
	using Base::Base;
};

class ste_shader_variable_layout_verification_type_mismatch : public ste_shader_variable_layout_verification_exception {
	using Base = ste_shader_variable_layout_verification_exception;

public:
	using Base::Base;
};

class ste_shader_variable_layout_verification_array_length_mismatch : public ste_shader_variable_layout_verification_exception {
	using Base = ste_shader_variable_layout_verification_exception;

public:
	using Base::Base;
};

class ste_shader_variable_layout_verification_array_stride_mismatch : public ste_shader_variable_layout_verification_exception {
	using Base = ste_shader_variable_layout_verification_exception;

public:
	using Base::Base;
};

class ste_shader_variable_layout_verification_offset_mismatch : public ste_shader_variable_layout_verification_exception {
	using Base = ste_shader_variable_layout_verification_exception;

public:
	using Base::Base;
};

}
}
