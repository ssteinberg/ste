//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vk_result.hpp>

#include <stdexcept>

namespace StE {
namespace GL {

class ste_shader_load_exception : public std::runtime_error {
	using Base = std::runtime_error;

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
