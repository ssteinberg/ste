//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vk_result.hpp>

#include <ste_engine_exceptions.hpp>

namespace StE {
namespace GL {

class device_memory_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

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
