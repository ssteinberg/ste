//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <rendering_system.hpp>
#include <fragment.hpp>

#include <device_pipeline_shader_stage.hpp>
#include <pipeline_auditor_compute.hpp>
#include <device_pipeline_compute.hpp>

#include <string>

namespace ste {
namespace gl {

/**
*	@brief	A rendering system fragment with a compute pipeline
*/
class fragment_compute : public fragment {
private:
	device_pipeline_shader_stage shader_stage;

protected:
	device_pipeline_compute pipeline;

private:
	static auto create_compute_pipeline(const ste_context &ctx,
										pipeline_binding_set_pool &binding_set_pool,
										const pipeline_external_binding_set_collection* external_binding_sets_collection,
										device_pipeline_shader_stage &shader_stage) {
		// Compute pipeline auditor
		pipeline_auditor_compute auditor(shader_stage);

		// Create pipeline
		return external_binding_sets_collection ?
			auditor.pipeline(ctx,
							 binding_set_pool,
							 *external_binding_sets_collection) :
			auditor.pipeline(ctx,
							 binding_set_pool);
	}

protected:
	fragment_compute(const rendering_system &rs,
					 pipeline_binding_set_pool &binding_set_pool,
					 const std::string &shader_stage_name)
		: shader_stage(rs.get_creating_context(), shader_stage_name),
		pipeline(create_compute_pipeline(rs.get_creating_context(),
										 binding_set_pool,
										 rs.external_binding_sets(),
										 this->shader_stage))
	{}

public:
	virtual ~fragment_compute() noexcept {}

	// Subclasses are expected to declare:
	//static const std::string& name();
};

}
}
