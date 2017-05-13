//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image.hpp>
#include <image_view.hpp>
#include <image_layout.hpp>

#include <texture.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A packaged combined_image_sampler is a helper object that creates an image object and a packaged_texture with thatimage.
*			Used in pipeline binding set layouts for sampled-image/storage-image bindings.
*/
template <image_type type>
class packaged_texture : public texture_generic, public allow_type_decay<packaged_texture<type>, texture<type>> {
private:
	static constexpr auto dimensions = image_dimensions_v<type>;

private:
	device_image<dimensions> image;
	texture<type> t;

public:
	packaged_texture(device_image<dimensions> &&image,
					 image_layout layout)
		: image(std::move(image)),
		t(image_view<type>(this->image),
		  layout)
	{}

	packaged_texture(packaged_texture&&) = default;
	packaged_texture &operator=(packaged_texture&& o) = default;

	VkImageView get_image_view_handle() const override final { return t.get_image_view_handle(); }
	image_layout get_layout() const override final { return t.get_layout(); }

	auto& get_image_view() const { return t.get_image_view(); }
	auto& get_image() const { return image; }
	auto& get() const { return t; }
};

}
}
