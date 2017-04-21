//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_bind_point_base.hpp>
#include <device_pipeline_exceptions.hpp>

#include <pipeline_layout.hpp>

#include <string>
#include <types.hpp>

namespace StE {
namespace GL {

class pipeline_specialization_constant_bind_point : public pipeline_bind_point_base {
	using Base = pipeline_bind_point_base;

private:
	pipeline_layout *layout;
	std::string name;

private:
	/**
	*	@brief	Specializes constant at a binding point to a value
	*
	*	@param	value		Value to specialize to. Must be a POD.
	*/
	template <typename T>
	void specialize_constant(const T &value) {
		layout->specialize_constant(name, value);
	}
	/**
	*	@brief	Remove a specialization of constant at a binding point
	*/
	void remove_specialization() {
		layout->remove_specialization(name);
	}

public:
	pipeline_specialization_constant_bind_point(const ste_shader_stage_binding_variable *variable,
												pipeline_layout *layout,
												const std::string &name)
		: Base(variable),
		layout(layout), 
		name(name)
	{}
	~pipeline_specialization_constant_bind_point() noexcept {}

	pipeline_specialization_constant_bind_point(pipeline_specialization_constant_bind_point&&) = default;
	pipeline_specialization_constant_bind_point &operator=(pipeline_specialization_constant_bind_point&&) = default;

	template <typename T>
	static void errorneous_assign() {
		// Called by base class dispatcher when an incompatible type is passed to operator=
		throw device_pipeline_incompatible_bind_type_exception("T can not be bound to this bind point");
	}

	/**
	*	@brief	Unbind specialization constant (resetting it to its default value)
	*/
	void operator=(const none_t&) {
		this->remove_specialization();
	}

	/**
	*	@brief	Sets a specialization constant
	*/
	template <
		typename T,
		typename = typename std::enable_if<!is_none_v<T> && std::is_pod_v<T>>::type
	>
	void operator=(T&& t) {
		this->specialize_constant(std::forward<T>(t));
	}
};

}
}
