//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <texture.hpp>

#include <device_image.hpp>
#include <image_view.hpp>
#include <sampler.hpp>

#include <image_layout.hpp>
#include <image_type_traits.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A packaged texture is a helper object that creates an image object, an image view of that image and the
*			texture object referencing that image view.
*			Used in pipeline binding set layouts for combined-image-sampler bindings.
*/
template <image_type type>
class packaged_texture : public texture_generic, public allow_type_decay<packaged_texture<type>, texture<type>> {
private:
	static constexpr auto dimensions = image_dimensions_v<type>;

private:
	device_image<dimensions> image;
	texture<type> tex;

private:
	auto create_view(const image_view_swizzle &swizzle) {
		return image_view<type>(image,
								swizzle);
	}

public:
	packaged_texture(device_image<dimensions> &&image,
					 const sampler *samp,
					 image_layout layout,
					 const image_view_swizzle &swizzle = {})
		: image(std::move(image)),
		tex(make_texture<type>(create_view(swizzle),
							   samp,
							   layout))
	{}
	packaged_texture(device_image<dimensions> &&image,
					 sampler &&samp,
					 image_layout layout,
					 const image_view_swizzle &swizzle = {})
		: image(std::move(image)),
		tex(make_texture<type>(create_view(swizzle),
							   samp,
							   layout))
	{}

	packaged_texture(packaged_texture&&) = default;
	packaged_texture &operator=(packaged_texture&&) = default;

	const sampler& get_sampler() const override final { return tex.get_sampler(); }
	VkImageView get_image_view_handle() const override final { return tex.get_image_view_handle(); }
	image_layout get_layout() const override final { return tex.get_layout(); }

	auto& get_image_view() const { return tex.get_image_view(); }

	auto& get() const { return tex; }
};

}
}
