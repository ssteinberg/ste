//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_engine_exceptions.hpp>

namespace ste {
namespace gl {

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

class device_pipeline_incompatible_binding_sets_exception : public device_pipeline_exception {
	using Base = device_pipeline_exception;

public:
	using Base::Base;
};

class device_pipeline_incompatible_bind_type_exception : public device_pipeline_exception {
	using Base = device_pipeline_exception;

public:
	using Base::Base;
};

class device_pipeline_no_attached_framebuffer_exception : public device_pipeline_exception {
	using Base = device_pipeline_exception;

public:
	using Base::Base;
};

class device_pipeline_incompatible_framebuffer_exception : public device_pipeline_exception {
	using Base = device_pipeline_exception;

public:
	using Base::Base;
};

}
}
