//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_engine_exceptions.hpp>

namespace ste {
namespace gl {

class ste_device_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

public:
	using Base::Base;
};

class ste_device_creation_exception : public ste_device_exception {
	using Base = ste_device_exception;

public:
	using Base::Base;
};

class ste_device_presentation_unsupported_exception : public ste_device_exception {
	using Base = ste_device_exception;

public:
	using Base::Base;
};

class ste_device_not_queue_thread_exception : public ste_device_exception {
	using Base = ste_device_exception;

public:
	ste_device_not_queue_thread_exception() : Base("Method requires executing on a ste_device_queue thread") {}
	using Base::Base;
};

}
}
