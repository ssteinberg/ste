//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vk_result.hpp>

#include <ste_engine_exceptions.hpp>

namespace ste {
namespace gl {

class ste_shader_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

public:
	using Base::Base;
};

class ste_shader_load_unrecognized_exception : public ste_shader_exception {
	using Base = ste_shader_exception;

public:
	using Base::Base;
	ste_shader_load_unrecognized_exception() : Base("Loaded shader binary is unsupported or unrecognized") {}
};

class ste_shader_load_spirv_corrupt_or_incompatible : public ste_shader_exception {
	using Base = ste_shader_exception;

public:
	using Base::Base;
	ste_shader_load_spirv_corrupt_or_incompatible() : Base("Loaded shader binary contains corrupted or incompatible SPIR-v module") {}
};

class ste_shader_output_attachment_at_non_fragment_shader_stage_exception : public ste_shader_exception {
	using Base = ste_shader_exception;

public:
	using Base::Base;
	ste_shader_output_attachment_at_non_fragment_shader_stage_exception() : Base("Only fragment shaders can have output attachments") {}
};

class ste_shader_opaque_or_unknown_type : public ste_shader_exception {
	using Base = ste_shader_exception;

public:
	using Base::Base;
	ste_shader_opaque_or_unknown_type() : Base("Type is opaque or unknown") {}
};

class ste_shader_binding_incompatible_type : public ste_shader_exception {
	using Base = ste_shader_exception;

public:
	using Base::Base;
};

class ste_shader_binding_specialization_or_push_constant_exception : public ste_shader_exception {
	using Base = ste_shader_exception;

public:
	using Base::Base;
};

}
}
