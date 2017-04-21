//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_external_binding_layout.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_resource_binder.hpp>

#include <string>

namespace StE {
namespace GL {

class pipeline_external_resource_bind_point {
private:
	pipeline_resource_binding_queue *queue;
	std::string name;
	const pipeline_external_binding_layout *binding;

private:
	template <typename WriteType, typename T>
	void bind(const pipeline_resource_binder<WriteType, T> &res) {
		using resource_type = pipeline_resource_binder<WriteType, T>;
		using resource_underlying_type = typename resource_type::UnderlyingType;

		// Validate
		binding->validate_layout<resource_underlying_type>();

		// Generate write descriptor, and append to queue
		auto set_idx = binding->set_idx();
		auto write = res.writer(binding);
		(*queue)[set_idx].push_back(write);
	}

public:
	pipeline_external_resource_bind_point(pipeline_resource_binding_queue *q,
										  const std::string &name,
										  const pipeline_external_binding_layout *binding)
		: queue(q), name(name), binding(binding)
	{}
	~pipeline_external_resource_bind_point() noexcept {}

	pipeline_external_resource_bind_point(pipeline_external_resource_bind_point&&) = default;
	pipeline_external_resource_bind_point &operator=(pipeline_external_resource_bind_point&&) = default;

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
