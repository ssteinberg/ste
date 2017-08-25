//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_logical_device.hpp>

#include <optional.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

struct vk_sampler_info {
	VkFilter                mag_filter;
	VkFilter                min_filter;
	VkSamplerMipmapMode     mipmap_mode;
	VkSamplerAddressMode    address_mode_u;
	VkSamplerAddressMode    address_mode_v;
	VkSamplerAddressMode    address_mode_w;
	float                   mip_lod_bias;
	VkBool32                anisotropy_enable;
	float                   max_anisotropy;
	VkBool32                compare_enable;
	VkCompareOp             compare_op;
	float                   min_lod;
	float                   max_lod;
	VkBorderColor           border_color;
	VkBool32                unnormalized_coordinates;

	operator VkSamplerCreateInfo() const {
		VkSamplerCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.magFilter = mag_filter;
		create_info.minFilter = min_filter;
		create_info.mipmapMode = mipmap_mode;
		create_info.addressModeU = address_mode_u;
		create_info.addressModeV = address_mode_v;
		create_info.addressModeW = address_mode_w;
		create_info.mipLodBias = mip_lod_bias;
		create_info.anisotropyEnable = anisotropy_enable;
		create_info.maxAnisotropy = max_anisotropy;
		create_info.compareEnable = compare_enable;
		create_info.compareOp = compare_op;
		create_info.minLod = min_lod;
		create_info.maxLod = max_lod;
		create_info.borderColor = border_color;
		create_info.unnormalizedCoordinates = unnormalized_coordinates;

		return create_info;
	}
};

template <typename host_allocator = vk_host_allocator<>>
class vk_sampler : public allow_type_decay<vk_sampler<host_allocator>, VkSampler> {
private:
	optional<VkSampler> sampler;
	alias<const vk_logical_device<host_allocator>> device;

public:
	vk_sampler(const vk_logical_device<host_allocator> &device,
			   const vk_sampler_info &info) : device(device) {
		VkSamplerCreateInfo create_info = info;

		VkSampler sampler;
		vk_result res = vkCreateSampler(device, &create_info, &host_allocator::allocation_callbacks(), &sampler);
		if (!res) {
			throw vk_exception(res);
		}

		this->sampler = sampler;
	}
	~vk_sampler() noexcept {
		destroy_sampler();
	}

	vk_sampler(vk_sampler &&) = default;
	vk_sampler &operator=(vk_sampler &&o) noexcept {
		destroy_sampler();

		sampler = std::move(o.sampler);
		device = std::move(o.device);

		return *this;
	}
	vk_sampler(const vk_sampler &) = delete;
	vk_sampler &operator=(const vk_sampler &) = delete;

	void destroy_sampler() {
		if (sampler) {
			vkDestroySampler(device.get(), *this, &host_allocator::allocation_callbacks());
			sampler = none;
		}
	}

	auto& get() const { return sampler.get(); }
};

}

}
}
