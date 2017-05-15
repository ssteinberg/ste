//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <combined_image_sampler.hpp>

#include <device_image.hpp>
#include <image_view.hpp>
#include <sampler.hpp>

#include <image_layout.hpp>
#include <image_type_traits.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A packaged combined_image_sampler is a helper object that creates an image object, an image view of that image and the
*			combined_image_sampler object referencing that image view.
*			Used in pipeline binding set layouts for combined-image-sampler bindings.
*/
template <image_type type>
class packaged_image_sampler : public combined_image_sampler_generic, public allow_type_decay<packaged_image_sampler<type>, combined_image_sampler<type>> {
private:
	static constexpr auto dimensions = image_dimensions_v<type>;

private:
	device_image<dimensions> image;
	combined_image_sampler<type> cis;

private:
	auto create_view(const image_view_swizzle &swizzle) {
		return image_view<type>(image,
								image.get_format(),
								swizzle);
	}

public:
	packaged_image_sampler(device_image<dimensions> &&image,
						   const sampler *samp,
						   image_layout layout,
						   const image_view_swizzle &swizzle = {})
		: image(std::move(image)),
		cis(make_combined_image_sampler<type>(create_view(swizzle),
											  samp,
											  layout))
	{}
	packaged_image_sampler(device_image<dimensions> &&image,
						   sampler &&samp,
						   image_layout layout,
						   const image_view_swizzle &swizzle = {})
		: image(std::move(image)),
		cis(make_combined_image_sampler<type>(create_view(swizzle),
											  samp,
											  layout))
	{}

	packaged_image_sampler(packaged_image_sampler&&) = default;
	packaged_image_sampler &operator=(packaged_image_sampler&&) = default;

	VkImageView get_image_view_handle() const override final { return cis.get_image_view_handle(); }
	image_layout get_layout() const override final { return cis.get_layout(); }

	const sampler& get_sampler() const override final { return cis.get_sampler(); }
	auto& get_image_view() const { return cis.get_image_view(); }
	auto& get_image() const { return image; }
	auto& get() const { return cis; }

	auto get_texture() const {
		return get().get_texture();
	}
};

}
}
