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
	template <typename WriteType, typename T>
	void bind(lib::vector<pipeline_resource_binder<WriteType, T>> &&resources) {
		using resource_type = pipeline_resource_binder<WriteType, T>;
		using resource_underlying_type = typename resource_type::UnderlyingType;

		auto set_idx = binding->set_idx();
		auto& q = (*queue)[set_idx];

		// Validate
		get_binding().validate_layout<resource_underlying_type>();

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
	void operator=(lib::vector<pipeline_resource_binder<WriteType, T>> &&resources) {
		this->bind(std::move(resources));
	}

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
