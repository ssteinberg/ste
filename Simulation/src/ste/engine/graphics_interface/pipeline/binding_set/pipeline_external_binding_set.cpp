
#include <stdafx.hpp>
#include <pipeline_external_binding_set.hpp>

#include <pipeline_external_resource_bind_point.hpp>

using namespace ste;
using namespace ste::gl;

pipeline_external_resource_bind_point pipeline_external_binding_set::operator[](const lib::string &resource_name) {
	const pipeline_external_binding_layout *bind;

	auto var_it = name_map.find(resource_name);
	if (var_it != name_map.end()) {
		// Name references a variable
		bind = var_it->second;
	}
	else {
		throw device_pipeline_unrecognized_variable_name_exception("Resource with provided name doesn't exist in pipeline layout");
	}

	// Create the binder
	return pipeline_external_resource_bind_point(&binding_queue,
												 resource_name,
												 bind,
												 this);
}
