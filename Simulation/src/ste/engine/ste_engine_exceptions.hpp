//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdexcept>
#include <lib/string.hpp>

namespace ste {

class ste_engine_exception : public std::runtime_error {
	using Base = std::runtime_error;

public:
	ste_engine_exception(const lib::string &str) : Base(str.c_str()) {}
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
