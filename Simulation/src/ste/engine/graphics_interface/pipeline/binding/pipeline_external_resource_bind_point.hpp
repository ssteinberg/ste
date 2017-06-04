//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
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

private:
	template <typename WriteType, typename T>
	void bind(lib::vector<pipeline_resource_binder<WriteType, T>> &&resources) {
		using resource_type = pipeline_resource_binder<WriteType, T>;
		using resource_underlying_type = typename resource_type::UnderlyingType;

		auto set_idx = binding->set_idx();
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

public:
	pipeline_external_resource_bind_point(pipeline_resource_binding_queue *q,
										  const lib::string &name,
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
	void operator=(pipeline_resource_binder<WriteType, T> &&res) {
		this->bind(std::move(res));
	}
};

}
}
