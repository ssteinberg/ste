//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <pipeline_auditor.hpp>
#include <pipeline_layout.hpp>
#include <pipeline_external_binding_set_collection.hpp>

#include <device_pipeline_shader_stage.hpp>
#include <pipeline_layout_exceptions.hpp>

#include <device_pipeline_compute.hpp>

#include <vector>

namespace ste {
namespace gl {

/**
*	@brief	Describes the compute pipeline configuration.
*/
class pipeline_auditor_compute : public pipeline_auditor {
	using Base = pipeline_auditor;

private:
	shader_stage_t compute_shader_stage{ nullptr };

public:
	pipeline_auditor_compute() = default;
	pipeline_auditor_compute(device_pipeline_shader_stage &compute_stage) {
		attach_shader_stage(compute_stage);
	}
	~pipeline_auditor_compute() noexcept {}

	pipeline_auditor_compute(pipeline_auditor_compute&&) = default;
	pipeline_auditor_compute &operator=(pipeline_auditor_compute&&) = default;

	/**
	*	@brief	Attaches a shader stage.
	*
	*	@throws	pipeline_layout_incompatible_stage_exception	If provided stage is not a compute shader stage
	*/
	void attach_shader_stage(device_pipeline_shader_stage &stage) {
		if (stage.get_stage() != ste_shader_program_stage::compute_program) {
			throw pipeline_layout_incompatible_stage_exception("Excepted a compute shader stage.");
		}
		compute_shader_stage = &stage;
	}

	/**
	*	@brief	Generates a pipeline from the specifications recorded to the auditor.
	*
	*	@param	ctx				Context
	*	@param	external_binding_sets	A list of binding set layouts that are assumed to be created and bound by an external system.
	*									The pipeline will only check compatibility with the provided shader stages.
	*/
	auto pipeline(const ste_context &ctx,
				  optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets) const {
		pipeline_layout layout(ctx,
							   stages(),
							   external_binding_sets);
		return device_pipeline_compute(device_pipeline_compute::ctor(),
									   ctx,
									   std::move(layout),
									   external_binding_sets);
	}
	/**
	 *	@brief	See pipeline().
	 */
	auto pipeline(const ste_context &ctx) const {
		return pipeline(ctx,
						none);
	}

private:
	/**
	*	@brief	Generates the list of pipeline stages
	*/
	std::vector<shader_stage_t> stages() const override final {
		std::vector<shader_stage_t> stages;
		if (compute_shader_stage != nullptr)
			stages.push_back(compute_shader_stage);

		return stages;
	}
};

}
}
