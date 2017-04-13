//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_engine_exceptions.hpp>

namespace StE {
namespace GL {

class device_pipeline_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

public:
	using Base::Base;
};

class device_pipeline_unrecognized_variable_name_exception : public device_pipeline_exception {
	using Base = device_pipeline_exception;

public:
	using Base::Base;
};

class device_pipeline_only_specialization_constants_can_be_unbound : public device_pipeline_exception {
	using Base = device_pipeline_exception;

public:
	using Base::Base;
};

class device_pipeline_attempting_to_bind_incompatible_type_exception : public device_pipeline_exception {
	using Base = device_pipeline_exception;

public:
	using Base::Base;
};

}
}
