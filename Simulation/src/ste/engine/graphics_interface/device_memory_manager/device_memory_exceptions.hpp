//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vk_result.hpp>

#include <stdexcept>

namespace StE {
namespace GL {

class device_memory_exception : public std::runtime_error {
	using Base = std::runtime_error;

public:
	using Base::Base;
};

class device_memory_allocation_failed : public device_memory_exception {
	using Base = device_memory_exception;

public:
	using Base::Base;
	device_memory_allocation_failed() : Base("") {}
};

class device_memory_no_supported_heap_exception : public device_memory_exception {
	using Base = device_memory_exception;

public:
	using Base::Base;
	device_memory_no_supported_heap_exception() : Base("") {}
};

}
}
