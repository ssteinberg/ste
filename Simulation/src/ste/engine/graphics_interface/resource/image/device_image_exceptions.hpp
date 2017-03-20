//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_engine_exceptions.hpp>

namespace StE {

class device_image_format_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

public:
	using Base::Base;
};

}
