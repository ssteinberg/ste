//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <pipeline_layout.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_resource_bind_point.hpp>
#include <device_pipeline_exceptions.hpp>

#include <pipeline_layout_set_index.hpp>
#include <pipeline_binding_set_collection.hpp>
#include <pipeline_binding_set_pool.hpp>

#include <boost/container/flat_map.hpp>

namespace StE {
namespace GL {

class device_pipeline {
protected:
	const ste_context &ctx;

	pipeline_layout layout;
	pipeline_resource_binding_queue binding_queue;

	pipeline_binding_set_collection binding_sets;

private:
	void on_bind() {
		
	}

protected:
	device_pipeline(const ste_context &ctx,
					pipeline_binding_set_pool &pool,
					pipeline_layout &&layout)
		: ctx(ctx),
		layout(std::move(layout)),
		binding_sets(this->layout, 
					 pool)
	{}

public:
	device_pipeline(device_pipeline&&) = default;
	device_pipeline &operator=(device_pipeline&&) = default;

	~device_pipeline() noexcept {}

	/**
	 *	@brief	Creates a resource binder for a given variable name
	 */
	auto operator[](const std::string &resource_name) {
		const pipeline_binding_set_layout_binding *bind = nullptr;
		
		auto var_it = layout.variables_map.find(resource_name);
		if (var_it != layout.variables_map.end()) {
			// Name references a variable
			bind = &var_it->second;
		}
		else {
			throw device_pipeline_unrecognized_variable_name_exception("Resource with provided name doesn't exist in pipeline layout");
		}

		// Create the binder
		return pipeline_resource_bind_point(&binding_queue,
											&layout,
											resource_name,
											bind);
	}
};

}
}
