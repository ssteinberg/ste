//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_logical_device.hpp>
#include <vk_host_allocator.hpp>

#include <vulkan/vulkan.h>
#include <optional.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_pipeline : public allow_type_decay<vk_pipeline<host_allocator>, VkPipeline> {
private:
	alias<const vk_logical_device<host_allocator>> device;

protected:
	optional<VkPipeline> pipeline;

protected:
	vk_pipeline(const vk_logical_device<host_allocator> &device)
		: device(device)
	{}

public:
	virtual ~vk_pipeline() noexcept {
		destroy_pipeline();
	}

	vk_pipeline(vk_pipeline&&) = default;
	vk_pipeline &operator=(vk_pipeline&&o) noexcept {
		destroy_pipeline();

		pipeline = std::move(o.pipeline);
		device = std::move(o.device);

		return *this;
	}
	vk_pipeline(const vk_pipeline &) = delete;
	vk_pipeline &operator=(const vk_pipeline &) = delete;

	void destroy_pipeline() {
		if (pipeline) {
			vkDestroyPipeline(device.get(), *this, &host_allocator::allocation_callbacks());
			pipeline = none;
		}
	}

	auto& get() const { return pipeline.get(); }
};

}

}
}
