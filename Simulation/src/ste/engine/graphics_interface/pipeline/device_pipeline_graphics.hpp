//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_pipeline.hpp>

namespace StE {
namespace GL {

class device_pipeline_graphics : public device_pipeline {
	using Base = device_pipeline;

	friend class pipeline_auditor_graphics;

private:
	struct ctor {};

protected:
	VkPipelineBindPoint pipeline_bind_point() const override final {
		return VK_PIPELINE_BIND_POINT_GRAPHICS;
	}

	void bind_pipeline(const command_buffer &, command_recorder &recorder) const override final {
//		recorder << cmd_bind_pipeline(graphics_pipeline);
	}

	void recreate_pipeline() override final {
	}

public:
	device_pipeline_graphics(ctor,
							 const ste_context &ctx,
							 pipeline_binding_set_pool &pool,
							 pipeline_layout &&layout,
							 optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets)
		: Base(ctx,
			   pool,
			   std::move(layout),
			   external_binding_sets)
	{}
	~device_pipeline_graphics() noexcept {}
};

}
}
