// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdexcept>

class atmospherics_lut_error : public std::runtime_error {
	using Base = std::runtime_error;

public:
	using Base::Base;
	atmospherics_lut_error() : Base("") {}
};
