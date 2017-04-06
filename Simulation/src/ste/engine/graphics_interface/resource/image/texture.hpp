//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image_base.hpp>
#include <sampler.hpp>

namespace StE {
namespace GL {

class texture {
private:
	std::reference_wrapper<const device_image_base> image;
	std::reference_wrapper<const sampler> sam;

public:
	texture(const device_image_base &image,
				   const sampler &sam)
		: image(image),
		sam(sam)
	{}

	auto& get_sampler() const { return sam.get(); }
	auto& get_image() const { return image.get(); }

	texture(texture&&) = default;
	texture &operator=(texture&&) = default;
};

}
}
