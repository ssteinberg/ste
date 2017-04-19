//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_pipeline.hpp>

#include <vk_pipeline_compute.hpp>

#include <cmd_bind_pipeline.hpp>

namespace StE {
namespace GL {

class device_pipeline_compute : public device_pipeline {
	using Base = device_pipeline;

	friend class pipeline_auditor_compute;

private:
	struct ctor {};

private:
	vk_pipeline_compute compute_pipeline;

private:
	auto create_pipeline_object() const {
		auto shader_stage_descriptors = layout.shader_stage_descriptors();
		return vk_pipeline_compute(ctx.device(),
								   shader_stage_descriptors,
								   layout);
	}

protected:
	VkPipelineBindPoint pipeline_bind_point() const override final {
		return VK_PIPELINE_BIND_POINT_COMPUTE;
	}

	void bind_pipeline(const command_buffer &, command_recorder &recorder) const override final {
		recorder << cmd_bind_pipeline(compute_pipeline);
	}

	void recreate_pipeline() override final {
		compute_pipeline = create_pipeline_object();
	}

public:
	device_pipeline_compute(ctor,
							const ste_context &ctx,
							pipeline_binding_set_pool &pool,
							optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets)
		: Base(ctx,
			   pool,
			   std::move(layout),
			   external_binding_sets),
		compute_pipeline(create_pipeline_object())
	{}
	~device_pipeline_compute() noexcept {}
};

}
}
