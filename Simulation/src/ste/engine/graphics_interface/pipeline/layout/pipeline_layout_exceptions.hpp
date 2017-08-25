//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_engine_exceptions.hpp>

namespace ste {
namespace gl {

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

class pipeline_layout_variable_not_found_in_external_set_exception : public pipeline_layout_exception {
	using Base = pipeline_layout_exception;

public:
	using Base::Base;
};

class pipeline_layout_external_set_duplicate_set_index : public pipeline_layout_exception {
	using Base = pipeline_layout_exception;

public:
	using Base::Base;
};

class pipeline_layout_variable_incompatible_with_external_set_exception : public pipeline_layout_exception {
	using Base = pipeline_layout_exception;

public:
	using Base::Base;
};

class pipeline_layout_push_constant_in_external_set_exception : public pipeline_layout_exception {
	using Base = pipeline_layout_exception;

public:
	using Base::Base;
};

}
}
