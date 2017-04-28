//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <image_view.hpp>
#include <sampler.hpp>

#include <image_layout.hpp>

namespace ste {
namespace gl {

class texture_generic {
public:
	virtual ~texture_generic() noexcept {}

	virtual const sampler& get_sampler() const = 0;
	virtual VkImageView get_image_view_handle() const = 0;
	virtual image_layout get_layout() const = 0;
};

template <image_type type>
class texture : public texture_generic {
private:
	const image_view<type>* image;
	std::reference_wrapper<const sampler> sam;
	image_layout layout;

public:
	texture(const image_view<type>* image,
			const sampler &sam,
			image_layout layout)
		: image(image),
		sam(sam),
		layout(layout)
	{}

	const sampler& get_sampler() const override final { return sam.get(); }
	VkImageView get_image_view_handle() const override final { return image; }
	image_layout get_layout() const override final { return layout; }

	auto& get_image_view() const { return *image; }

	texture(texture&&) = default;
	texture &operator=(texture&&) = default;
};

}
}
