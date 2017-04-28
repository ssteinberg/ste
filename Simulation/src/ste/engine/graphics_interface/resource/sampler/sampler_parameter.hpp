//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <limits>
#include <vk_sampler.hpp>

#include <sampler_filter.hpp>
#include <sampler_mipmap_mode.hpp>
#include <sampler_address_mode.hpp>
#include <sampler_border_color.hpp>
#include <compare_op.hpp>

namespace ste {
namespace gl {

namespace sampler_parameter {

struct parameter {
	virtual ~parameter() noexcept {}
	virtual void operator()(vk::vk_sampler_info &create_info) const = 0;
};

struct filtering : parameter {
	sampler_filter mag{ sampler_filter::nearest };
	sampler_filter min{ sampler_filter::nearest };
	sampler_mipmap_mode mipmap{ sampler_mipmap_mode::nearest };

	filtering() = default;
	filtering(sampler_filter mag, sampler_filter min) : mag(mag), min(min) {}
	filtering(sampler_filter mag, sampler_filter min, sampler_mipmap_mode mipmap) : mag(mag), min(min), mipmap(mipmap) {}
	filtering(sampler_mipmap_mode mipmap) : mipmap(mipmap) {}

	void operator()(vk::vk_sampler_info &create_info) const override final {
		create_info.mag_filter = static_cast<VkFilter>(mag);
		create_info.min_filter = static_cast<VkFilter>(min);
		create_info.mipmap_mode = static_cast<VkSamplerMipmapMode>(mipmap);
	}
};

struct address_mode : parameter {
	sampler_address_mode u{ sampler_address_mode::repeat };
	sampler_address_mode v{ sampler_address_mode::repeat };
	sampler_address_mode w{ sampler_address_mode::repeat };
	sampler_border_color border_color{ sampler_border_color::opaque_black_float };

	address_mode() = default;
	address_mode(sampler_address_mode u) : u(u) {}
	address_mode(sampler_address_mode u, sampler_address_mode v) : u(u), v(v) {}
	address_mode(sampler_address_mode u, sampler_address_mode v, sampler_address_mode w) : u(u), v(v), w(w) {}
	address_mode(sampler_address_mode u, sampler_address_mode v, sampler_address_mode w,
				 sampler_border_color border_color) : u(u), v(v), w(w), border_color(border_color) {}
	address_mode(sampler_border_color border_color) : border_color(border_color) {}

	void operator()(vk::vk_sampler_info &create_info) const override final {
		create_info.address_mode_u = static_cast<VkSamplerAddressMode>(u);
		create_info.address_mode_v = static_cast<VkSamplerAddressMode>(v);
		create_info.address_mode_w = static_cast<VkSamplerAddressMode>(w);
		create_info.border_color = static_cast<VkBorderColor>(border_color);
	}
};

struct anisotropy : parameter {
	bool enabled{ false };
	float max{ 1 };

	anisotropy() = default;
	anisotropy(float max) : enabled(true), max(max) {}

	void operator()(vk::vk_sampler_info &create_info) const override final {
		create_info.anisotropy_enable = enabled;
		create_info.max_anisotropy = max;
	}
};

struct depth_compare : parameter {
	bool enabled{ false };
	compare_op op{ compare_op::less };

	depth_compare() = default;
	depth_compare(compare_op op) : enabled(true), op(op) {}

	void operator()(vk::vk_sampler_info &create_info) const override final {
		create_info.compare_enable = enabled;
		create_info.compare_op = static_cast<VkCompareOp>(op);
	}
};

struct mipmap_lod : parameter {
	float bias{ 0 };
	float min{ .0f };
	float max{ std::numeric_limits<float>::max() };

	mipmap_lod() = default;
	mipmap_lod(float bias) : bias(bias) {}
	mipmap_lod(float min, float max) : min(min), max(max) {}
	mipmap_lod(float bias, float min, float max) : bias(bias), min(min), max(max) {}

	void operator()(vk::vk_sampler_info &create_info) const override final {
		create_info.mip_lod_bias = bias;
		create_info.min_lod = min;
		create_info.max_lod = max;
	}
};

struct unnormalized {
	sampler_filter filter{ sampler_filter::nearest };
	address_mode address_mode;
	mipmap_lod mipmap_lod;

	void operator()(vk::vk_sampler_info &create_info) const {
		create_info.unnormalized_coordinates = true;
		create_info.mag_filter = create_info.min_filter = static_cast<VkFilter>(filter);
		address_mode(create_info);
		mipmap_lod(create_info);
	}
};

}

}
}
