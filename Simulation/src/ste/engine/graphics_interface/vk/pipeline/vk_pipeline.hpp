//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_logical_device.hpp>

#include <vulkan/vulkan.h>
#include <optional.hpp>
#include <allow_type_decay.hpp>

namespace StE {
namespace GL {

class vk_pipeline : public allow_type_decay<vk_pipeline, VkPipeline> {
private:
	std::reference_wrapper<const vk_logical_device> device;

protected:
	optional<VkPipeline> pipeline;

protected:
	vk_pipeline(const vk_logical_device &device)
		: device(device)
	{}
	
public:
	virtual ~vk_pipeline() noexcept {
		destroy_pipeline();
	}

	vk_pipeline(vk_pipeline&&) = default;
	vk_pipeline &operator=(vk_pipeline&&) = default;

	void destroy_pipeline() {
		if (pipeline) {
			vkDestroyPipeline(device.get(), *this, nullptr);
			pipeline = none;
		}
	}

	auto& get() const { return pipeline.get(); }
};

}
}
