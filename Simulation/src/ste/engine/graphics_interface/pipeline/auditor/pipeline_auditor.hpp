//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_pipeline.hpp>
#include <pipeline_layout.hpp>

#include <pipeline_external_binding_set_collection.hpp>
#include <pipeline_binding_set_layout.hpp>
#include <pipeline_binding_set_pool.hpp>

#include <vector>
#include <optional.hpp>

namespace StE {
namespace GL {

class pipeline_auditor {
public:
	using shader_stage_t = pipeline_layout::shader_stage_t;

private:
	virtual std::vector<shader_stage_t> stages() const = 0;

public:
	pipeline_auditor() = default;
	virtual ~pipeline_auditor() noexcept {}

	/**
	 *	@brief	Generates a pipeline from the specifications recorded to the auditor.
	 *	
	 *	@param	ctx			Context
	 *	@param	pool			Binding set pool. Used to allocate the binding sets used by the pipeline.
	 *	@param	external_binding_sets	A list of binding set layouts that are assumed to be created and populated by an external system.
	 *									The pipeline will only check compatibility with the provided shader stages.
	 */
	virtual std::unique_ptr<device_pipeline> pipeline(const ste_context &ctx,
													  pipeline_binding_set_pool &pool,
													  optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets) const = 0;
	/**
	*	@brief	See pipeline()
	*/
	std::unique_ptr<device_pipeline> pipeline(const ste_context &ctx,
											  pipeline_binding_set_pool &pool) const {
		return pipeline(ctx,
						pool,
						none);
	}
};

}
}
