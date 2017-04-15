//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_layout.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_resource_binder.hpp>
#include <pipeline_binding_layout_collection.hpp>
#include <device_pipeline_exceptions.hpp>

#include <string>

namespace StE {
namespace GL {

class pipeline_resource_bind_point {
private:
	pipeline_resource_binding_queue *queue;
	pipeline_layout *layout;
	std::string name;
	const pipeline_binding_set_layout_binding *binding;

public:
	/**
	 *	@brief	Returns the binding point underlying binding layout
	 */
	auto& get_binding() const { return *binding->binding; }
	/**
	*	@brief	Returns the binding point underlying variable
	*/
	auto& get_var() const { return *binding->binding->variable; }

private:
	template <typename WriteType, typename T>
	void bind(const pipeline_resource_binder<WriteType, T> &res) {
		using resource_type = pipeline_resource_binder<WriteType, T>;
		using resource_underlying_type = typename resource_type::UnderlyingType;

		// Validate
		get_binding().validate_layout<resource_underlying_type>();

		// Generate write descriptor, and append to queue
		auto set_idx = binding->binding->set_idx;
		auto write = res.writer(binding);
		(*queue)[set_idx].push_back(write);
	}

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

	template <typename T>
	void set_push_value(T&& t) {
		// TODO
		assert(false);
	}

private:
	bool is_push() const {
		return get_binding().binding_type == ste_shader_stage_binding_type::push_constant;
	}
	bool is_spec() const {
		return get_binding().binding_type == ste_shader_stage_binding_type::spec_constant;
	}

public:
	pipeline_resource_bind_point(pipeline_resource_binding_queue *q,
								 pipeline_layout *layout,
								 const std::string &name,
								 const pipeline_binding_set_layout_binding *binding)
		: queue(q), layout(layout), name(name), binding(binding)
	{}
	~pipeline_resource_bind_point() noexcept {}

	pipeline_resource_bind_point(pipeline_resource_bind_point&&) = default;
	pipeline_resource_bind_point &operator=(pipeline_resource_bind_point&&) = default;

	void operator=(const none_t&) {
		if (is_spec())
			this->remove_specialization();
		else
			throw device_pipeline_only_specialization_constants_can_be_unbound("Only specialization constants can be unbound");
	}
	template <
		typename T,
		typename = typename std::enable_if_t<std::is_pod_v<T> && !is_none_v<T>>
	>
	void operator=(T&& t) {
		if (is_spec())
			this->specialize_constant(std::forward<T>(t));
		else if (is_push())
			this->set_push_value(std::forward<T>(t));
		else
			throw device_pipeline_attempting_to_bind_incompatible_type_exception("Expected a pipeline_resource_binder object");
	}
	template <typename WriteType, typename T>
	void operator=(const pipeline_resource_binder<WriteType, T> &res) {
		if (is_spec() || is_push())
			throw device_pipeline_attempting_to_bind_incompatible_type_exception("Expected a POD");

		this->bind(res);
	}

	auto* operator->() const { return &get_var(); }
};

}
}
