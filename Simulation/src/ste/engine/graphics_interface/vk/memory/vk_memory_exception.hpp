//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vk_exception.hpp>

namespace StE {
namespace GL {

class vk_memory_allocation_failed_exception : public vk_exception {
	using Base = vk_exception;

public:
	using Base::Base;
	vk_memory_allocation_failed_exception() : Base("") {}
};

class vk_memory_no_supported_heap_exception : public vk_exception {
	using Base = vk_exception;

public:
	using Base::Base;
	vk_memory_no_supported_heap_exception() : Base("") {}
};

}
}
