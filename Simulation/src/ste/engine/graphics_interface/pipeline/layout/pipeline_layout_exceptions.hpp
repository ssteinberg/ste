//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_engine_exceptions.hpp>

namespace StE {
namespace GL {

class pipeline_layout_exception : public ste_engine_exception {
	using Base = ste_engine_exception;

public:
	using Base::Base;
};

class pipeline_layout_incompatible_stage_exception : public pipeline_layout_exception {
	using Base = pipeline_layout_exception;

public:
	using Base::Base;
};

class pipeline_layout_incompatible_binding_exception : public pipeline_layout_exception {
	using Base = pipeline_layout_exception;

public:
	using Base::Base;
};

class pipeline_layout_duplicate_variable_name_exception : public pipeline_layout_exception {
	using Base = pipeline_layout_exception;

public:
	using Base::Base;
};

class pipeline_layout_duplicate_incompatible_overlapping_push_constants_exception : public pipeline_layout_exception {
	using Base = pipeline_layout_exception;

public:
	using Base::Base;
};

class pipeline_layout_variable_not_found_exception : public pipeline_layout_exception {
	using Base = pipeline_layout_exception;

public:
	using Base::Base;
};

}
}
