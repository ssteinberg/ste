//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <limits>

namespace ste {
namespace gl {

namespace vk {

struct vk_sampler_parameters {
	virtual ~vk_sampler_parameters() noexcept {}
	virtual void operator()(VkSamplerCreateInfo &create_info) const = 0;
};

struct vk_sampler_filtering : vk_sampler_parameters {
	VkFilter mag{ VK_FILTER_NEAREST };
	VkFilter min{ VK_FILTER_NEAREST };
	VkSamplerMipmapMode mipmap{ VK_SAMPLER_MIPMAP_MODE_NEAREST };

	vk_sampler_filtering() = default;
	vk_sampler_filtering(VkFilter mag, VkFilter min) : mag(mag), min(min) {}
	vk_sampler_filtering(VkFilter mag, VkFilter min, VkSamplerMipmapMode mipmap) : mag(mag), min(min), mipmap(mipmap) {}
	vk_sampler_filtering(VkSamplerMipmapMode mipmap) : mipmap(mipmap) {}

	void operator()(VkSamplerCreateInfo &create_info) const override final {
		create_info.magFilter = mag;
		create_info.minFilter = min;
		create_info.mipmapMode = mipmap;
	}
};

struct vk_sampler_address_mode : vk_sampler_parameters {
	VkSamplerAddressMode u{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
	VkSamplerAddressMode v{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
	VkSamplerAddressMode w{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
	VkBorderColor border_color{ VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK };

	vk_sampler_address_mode() = default;
	vk_sampler_address_mode(VkSamplerAddressMode u) : u(u) {}
	vk_sampler_address_mode(VkSamplerAddressMode u, VkSamplerAddressMode v) : u(u), v(v) {}
	vk_sampler_address_mode(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w) : u(u), v(v), w(w) {}
	vk_sampler_address_mode(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w,
							VkBorderColor border_color) : u(u), v(v), w(w), border_color(border_color) {}
	vk_sampler_address_mode(VkBorderColor border_color) : border_color(border_color) {}

	void operator()(VkSamplerCreateInfo &create_info) const override final {
		create_info.addressModeU = u;
		create_info.addressModeV = v;
		create_info.addressModeW = w;
		create_info.borderColor = border_color;
	}
};

struct vk_sampler_anisotropy : vk_sampler_parameters {
	bool enabled{ false };
	float max{ 1 };

	vk_sampler_anisotropy() = default;
	vk_sampler_anisotropy(float max) : enabled(true), max(max) {}

	void operator()(VkSamplerCreateInfo &create_info) const override final {
		create_info.anisotropyEnable = enabled;
		create_info.maxAnisotropy = max;
	}
};

struct vk_sampler_depth_compare : vk_sampler_parameters {
	bool enabled{ false };
	VkCompareOp op{ VK_COMPARE_OP_LESS };

	vk_sampler_depth_compare() = default;
	vk_sampler_depth_compare(VkCompareOp op) : enabled(true), op(op) {}

	void operator()(VkSamplerCreateInfo &create_info) const override final {
		create_info.compareEnable = enabled;
		create_info.compareOp = op;
	}
};

struct vk_sampler_mipmap_lod : vk_sampler_parameters {
	float bias{ 0 };
	float min{ .0f };
	float max{ std::numeric_limits<float>::max() };

	vk_sampler_mipmap_lod() = default;
	vk_sampler_mipmap_lod(float bias) : bias(bias) {}
	vk_sampler_mipmap_lod(float min, float max) : min(min), max(max) {}
	vk_sampler_mipmap_lod(float bias, float min, float max) : bias(bias), min(min), max(max) {}

	void operator()(VkSamplerCreateInfo &create_info) const override final {
		create_info.mipLodBias = bias;
		create_info.minLod = min;
		create_info.maxLod = max;
	}
};

struct vk_sampler_unnormalized_parameters {
	VkFilter filter{ VK_FILTER_NEAREST };
	vk_sampler_address_mode address_mode;
	vk_sampler_mipmap_lod mipmap_lod;

	void operator()(VkSamplerCreateInfo &create_info) const {
		create_info.unnormalizedCoordinates = true;
		create_info.magFilter = create_info.minFilter = filter;
		address_mode(create_info);
		mipmap_lod(create_info);
	}
};

}

}
}
