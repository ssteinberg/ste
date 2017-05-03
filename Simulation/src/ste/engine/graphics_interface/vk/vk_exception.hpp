//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vk_result.hpp>

#include <stdexcept>

namespace ste {
namespace gl {

namespace vk {

class vk_exception : public std::runtime_error {
	using Base = std::runtime_error;

public:
	using Base::Base;
	vk_exception(const vk_result &result) : Base(result.string()) {}
};

}

}
}
