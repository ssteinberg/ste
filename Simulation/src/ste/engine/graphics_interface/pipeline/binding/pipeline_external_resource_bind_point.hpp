//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_pipeline_exceptions.hpp>
#include <pipeline_external_binding_set.hpp>

#include <pipeline_external_binding_layout.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_resource_binder.hpp>

#include <lib/string.hpp>

namespace ste {
namespace gl {

class pipeline_external_resource_bind_point {
private:
	pipeline_resource_binding_queue *queue;
	lib::string name;
	const pipeline_external_binding_layout *binding;

	pipeline_external_binding_set *external_binding_set_collection;

private:
	template <typename WriteType, typename T>
	void bind(lib::vector<pipeline_resource_binder<WriteType, T>> &&resources) {
		using resource_type = pipeline_resource_binder<WriteType, T>;
		using resource_underlying_type = typename resource_type::UnderlyingType;

		const auto set_idx = binding->set_idx();
		auto& q = (*queue)[set_idx];

		// Validate
		binding->validate_layout<resource_underlying_type>();

		for (auto &res : resources) {
			// Generate write descriptors
			auto write = res.writer(binding);
			// Append write descriptor to queue
			q.push_back(write);
		}
	}

	template <typename WriteType, typename T>
	void bind(pipeline_resource_binder<WriteType, T> &&res) {
		bind<WriteType, T>(lib::vector<pipeline_resource_binder<WriteType, T>>{ std::move(res) });
	}

	/**
	*	@brief	Specializes constant at a binding point to a value
	*
	*	@param	value		Value to specialize to. Must be a POD.
	*/
	template <typename T>
	void specialize_constant(const T &value) {
		external_binding_set_collection->specialize_constant(name, value);
	}
	/**
	*	@brief	Remove a specialization of constant at a binding point
	*/
	void remove_specialization() {
		external_binding_set_collection->remove_specialization(name);
	}

public:
	pipeline_external_resource_bind_point(pipeline_resource_binding_queue *q,
										  const lib::string &name,
										  const pipeline_external_binding_layout *binding,
										  pipeline_external_binding_set *external_binding_set_collection)
		: queue(q), 
		name(name), 
		binding(binding),
		external_binding_set_collection(external_binding_set_collection)
	{}
	~pipeline_external_resource_bind_point() noexcept {}

	pipeline_external_resource_bind_point(pipeline_external_resource_bind_point&&) = default;
	pipeline_external_resource_bind_point &operator=(pipeline_external_resource_bind_point&&) = default;

	/**
	*	@brief	Binds a resource
	*/
	template <typename WriteType, typename T>
	void operator=(lib::vector<pipeline_resource_binder<WriteType, T>> &&resources) {
		if (binding->get_binding().binding_type != ste_shader_stage_binding_type::storage &&
			binding->get_binding().binding_type != ste_shader_stage_binding_type::uniform) {
			throw device_pipeline_incompatible_bind_type_exception("T can not be bound to this bind point");
		}

		this->bind(std::move(resources));
	}

	/**
	*	@brief	Binds a resource
	*/
	template <typename WriteType, typename T>
	void operator=(pipeline_resource_binder<WriteType, T> &&res) {
		if (binding->get_binding().binding_type != ste_shader_stage_binding_type::storage &&
			binding->get_binding().binding_type != ste_shader_stage_binding_type::uniform) {
			throw device_pipeline_incompatible_bind_type_exception("T can not be bound to this bind point");
		}

		this->bind(std::move(res));
	}

	/**
	*	@brief	Unbind specialization constant (resetting it to its default value)
	*/
	void operator=(const none_t&) {
		if (binding->get_binding().binding_type != ste_shader_stage_binding_type::spec_constant) {
			throw device_pipeline_incompatible_bind_type_exception("T can not be bound to this bind point");
		}

		this->remove_specialization();
	}

	/**
	*	@brief	Sets a specialization constant
	*/
	template <
		typename T,
		typename S = std::remove_cv_t<std::remove_reference_t<T>>,
		typename = typename std::enable_if<!is_none_v<S> && (std::is_pod_v<S> || is_arithmetic_v<S>)>::type
	>
		void operator=(T&& t) {
		if (binding->get_binding().binding_type != ste_shader_stage_binding_type::spec_constant) {
			throw device_pipeline_incompatible_bind_type_exception("T can not be bound to this bind point");
		}

		this->specialize_constant(std::forward<T>(t));
	}
};

}
}
