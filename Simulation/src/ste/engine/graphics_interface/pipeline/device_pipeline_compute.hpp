//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_pipeline.hpp>

#include <vk_pipeline_compute.hpp>

#include <cmd_bind_pipeline.hpp>

#include <lib/string.hpp>
#include <optional.hpp>

namespace ste {
namespace gl {

class device_pipeline_compute : public device_pipeline {
	using Base = device_pipeline;

	friend class pipeline_auditor_compute;

	struct ctor {};

private:
	lib::string pipeline_name;
	vk::vk_pipeline_compute<> compute_pipeline;

private:
	auto create_pipeline_object() const {
		auto shader_stage_descriptors = get_layout().shader_stage_descriptors();
		return vk::vk_pipeline_compute<>(ctx.get().device(),
										 shader_stage_descriptors.front(),
										 get_layout(),
										 pipeline_name.data(),
										 &ctx.get().device().pipeline_cache().current_thread_cache());
	}

protected:
	VkPipelineBindPoint get_pipeline_vk_bind_point() const override final {
		return VK_PIPELINE_BIND_POINT_COMPUTE;
	}

	void bind_pipeline(const command_buffer &, command_recorder &recorder) const override final {
		recorder << cmd_bind_pipeline(compute_pipeline);
	}

	optional<vk::vk_pipeline<>> recreate_pipeline() override final {
		// Slice old pipeline, storing the old vk::vk_pipeline object.
		vk::vk_pipeline<> &old_pipeline_object = compute_pipeline;
		vk::vk_pipeline<> old_pipeline = std::move(old_pipeline_object);

		// Create new
		compute_pipeline = create_pipeline_object();

		// And return old
		return old_pipeline;
	}

public:
	device_pipeline_compute(ctor,
							const ste_context &ctx,
							lib::unique_ptr<pipeline_layout> &&layout,
							optional<std::reference_wrapper<const pipeline_external_binding_set>> external_binding_set,
							const char *name)
		: Base(ctx,
			   std::move(layout),
			   external_binding_set), 
		pipeline_name(name),
		compute_pipeline(create_pipeline_object())
	{}
	~device_pipeline_compute() noexcept {}

	device_pipeline_compute(device_pipeline_compute&&) = default;
	device_pipeline_compute &operator=(device_pipeline_compute&&) = default;
};

}
}
