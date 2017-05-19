//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_bind_point_base.hpp>
#include <device_pipeline_exceptions.hpp>

#include <pipeline_binding_layout.hpp>
#include <pipeline_layout.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_resource_binder.hpp>

namespace ste {
namespace gl {

class pipeline_resource_bind_point : public pipeline_bind_point_base {
	using Base = pipeline_bind_point_base;

private:
	pipeline_resource_binding_queue *queue;
	pipeline_layout *layout;
	const pipeline_binding_layout *binding;

protected:
	/**
	*	@brief	Returns the binding point underlying binding layout
	*/
	auto& get_binding() const { return *binding->binding; }

private:
	void populate_bound_resources(const lib::vector<vk::vk_descriptor_set_write_buffer> &writes,
								  std::uint32_t array_element,
								  stage_flag stages) {
		//! WIP
	}
	void populate_bound_resources(const lib::vector<vk::vk_descriptor_set_write_image> &writes,
								  std::uint32_t array_element,
								  stage_flag stages) {
		//! WIP
	}

	template <typename WriteType, typename T>
	void bind(const pipeline_resource_binder<WriteType, T> &res) {
		using resource_type = pipeline_resource_binder<WriteType, T>;
		using resource_underlying_type = typename resource_type::UnderlyingType;

		// Validate
		get_binding().validate_layout<resource_underlying_type>();

		// Generate write descriptor
		auto write = res.writer(binding);
		// Append write descriptor to queue
		auto set_idx = binding->set_idx();
		(*queue)[set_idx].push_back(write);

		// Insert resources into bound resources map
		auto array_element = write.get_array_element();
		stage_flag stages = binding->stages;
		populate_bound_resources(res.get_writes(),
								 array_element,
								 stages);
	}

public:
	pipeline_resource_bind_point(pipeline_resource_binding_queue *q,
								 pipeline_layout *layout,
								 const pipeline_binding_layout *binding)
		: Base(binding->binding->variable.get()),
		queue(q),
		layout(layout),
		binding(binding)
	{}
	~pipeline_resource_bind_point() noexcept {}

	pipeline_resource_bind_point(pipeline_resource_bind_point&&) = default;
	pipeline_resource_bind_point &operator=(pipeline_resource_bind_point&&) = default;

	template <typename T>
	static void errorneous_assign() {
		// Called by base class dispatcher when an incompatible type is passed to operator=
		throw device_pipeline_incompatible_bind_type_exception("T can not be bound to this bind point");
	}

	/**
	*	@brief	Binds a resource
	*/
	template <typename WriteType, typename T>
	void operator=(const pipeline_resource_binder<WriteType, T> &res) {
		this->bind(res);
	}
};

}
}
