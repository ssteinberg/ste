//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
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
template <typename CRTP, typename... ConsumedStorages>
class fragment_compute : public fragment<ConsumedStorages...> {
private:
	device_pipeline_shader_stage shader_stage;

protected:
	device_pipeline_compute pipeline;

private:
	static auto create_compute_pipeline(const ste_context &ctx,
										pipeline_binding_set_pool &binding_set_pool,
										device_pipeline_shader_stage &shader_stage) {
		// Compute pipeline auditor
		pipeline_auditor_compute auditor(shader_stage);

		// Create pipeline
		return auditor.pipeline(ctx,
								binding_set_pool);
	}

protected:
	fragment_compute(const ste_context &ctx,
					 pipeline_binding_set_pool &binding_set_pool,
					 const std::string &shader_stages_name)
		: shader_stage(ctx, shader_stages_name),
		pipeline(create_compute_pipeline(ctx,
										 binding_set_pool,
										 this->shader_stage))
	{}

public:
	virtual ~fragment_compute() noexcept {}

	// Subclasses are expected to declare:
	//static const std::string& name();
};

}
}
