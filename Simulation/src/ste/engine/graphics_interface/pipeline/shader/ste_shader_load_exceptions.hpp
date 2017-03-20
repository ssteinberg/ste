//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vk_result.hpp>

#include <ste_engine_exceptions.hpp>

namespace StE {
namespace GL {

class ste_shader_load_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

public:
	using Base::Base;
};

class ste_shader_load_unrecognized_exception : public ste_shader_load_exception {
	using Base = ste_shader_load_exception;

public:
	using Base::Base;
	ste_shader_load_unrecognized_exception() : Base("") {}
};

}
}
