//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_bind_point_base.hpp>
#include <device_pipeline_exceptions.hpp>

#include <pipeline_push_constants_layout.hpp>
#include <push_constant_descriptor.hpp>

namespace ste {
namespace gl {

class pipeline_push_constant_bind_point : public pipeline_bind_point_base {
	using Base = pipeline_bind_point_base;

private:
	pipeline_push_constants_layout *push_layout;
	const push_constant_descriptor *push_constant;

private:
	template <typename T>
	void set_push_value(T&& t) {
		push_layout->write_constant(std::forward<T>(t), 
									push_constant);
	}

public:
	pipeline_push_constant_bind_point(pipeline_push_constants_layout *push_layout,
									  const push_constant_descriptor *push_constant)
		: Base(&push_constant->get_var()),
		push_layout(push_layout), 
		push_constant(push_constant)
	{}
	~pipeline_push_constant_bind_point() noexcept {}

	pipeline_push_constant_bind_point(pipeline_push_constant_bind_point&&) = default;
	pipeline_push_constant_bind_point &operator=(pipeline_push_constant_bind_point&&) = default;

	template <typename T>
	static void errorneous_assign() {
		// Called by base class dispatcher when an incompatible type is passed to operator=
		throw device_pipeline_incompatible_bind_type_exception("T can not be bound to this bind point");
	}

	/**
	*	@brief	Sets a push constant
	*/
	template <typename T>
	void operator=(T&& t) {
		// Write push constant data
		this->set_push_value(std::forward<T>(t));
	}
};

}
}
