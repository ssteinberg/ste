//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_engine_exceptions.hpp>

namespace ste {

class ste_window_creation_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

public:
	using Base::Base;
};

}
