//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdexcept>

namespace StE {

class ste_window_creation_exception : public std::runtime_error {
	using Base = std::runtime_error;

public:
	using Base::Base;
};

}
