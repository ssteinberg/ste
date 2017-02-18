//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdexcept>

namespace StE {

class ste_engine_exception : public std::runtime_error {
	using Base = std::runtime_error;

public:
	using Base::Base;
};

class ste_engine_glfw_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

public:
	using Base::Base;
};

class ste_engine_creation_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

public:
	using Base::Base;
};

}
