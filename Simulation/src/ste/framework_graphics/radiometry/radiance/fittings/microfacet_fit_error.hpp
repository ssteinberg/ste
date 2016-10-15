// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdexcept>

class microfacet_fit_error : public std::runtime_error {
	using Base = std::runtime_error;

public:
	using Base::Base;
	microfacet_fit_error() : Base("") {}
};
