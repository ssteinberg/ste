//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <image_view.hpp>
#include <sampler.hpp>
#include <texture.hpp>

#include <image_layout.hpp>

#include <composite_resource_component_storage.hpp>

namespace ste {
namespace gl {

class combined_image_sampler_generic {
public:
	virtual ~combined_image_sampler_generic() noexcept {}

	virtual const sampler& get_sampler() const = 0;
	virtual VkImageView get_image_view_handle() const = 0;
	virtual image_layout get_layout() const = 0;
};

/**
 *	@brief	A combined_image_sampler is a combination of a sampler and an image view.
 *			Used in pipeline binding set layouts for combined-image-sampler bindings.
 */
template <image_type type>
class combined_image_sampler : public combined_image_sampler_generic {
private:
	_internal::composite_resource_component_storage<image_view<type>> view;
	_internal::composite_resource_component_storage<sampler> samp;
	image_layout layout;

public:
	combined_image_sampler(const image_view<type> *view,
						   const sampler *samp,
						   image_layout layout)
		: view(view),
		samp(samp),
		layout(layout)
	{}
	combined_image_sampler(image_view<type> &&view,
						   const sampler *samp,
						   image_layout layout)
		: view(std::move(view)),
		samp(samp),
		layout(layout)
	{}
	combined_image_sampler(const image_view<type> *view,
						   sampler &&samp,
						   image_layout layout)
		: view(view),
		samp(std::move(samp)),
		layout(layout)
	{}
	combined_image_sampler(image_view<type> &&view,
						   sampler &&samp,
						   image_layout layout)
		: view(std::move(view)),
		samp(std::move(samp)),
		layout(layout)
	{}

	combined_image_sampler(combined_image_sampler&&) = default;
	combined_image_sampler &operator=(combined_image_sampler&&) = default;

	VkImageView get_image_view_handle() const override final { return view.get().get_image_view_handle(); }
	image_layout get_layout() const override final { return layout; }

	const sampler& get_sampler() const override final { return samp.get(); }
	auto& get_image_view() const { return view.get(); }

	auto get_texture() const {
		return texture<type>(&view.get(),
							 layout);
	}
};

/**
 *	@brief	Constructs a new combined_image_sampler object, referencing an image and sampler.
 *			The referenced objects must not be moved or destroyed for the lifetime of the combined_image_sampler.
 */
template <image_type type>
auto make_combined_image_sampler(const image_view<type> *view,
								 const sampler *samp,
								 image_layout layout) {
	return combined_image_sampler<type>(view, samp, layout);
}

/**
*	@brief	Constructs a new combined_image_sampler object, referencing a sampler and taking ownership of an image.
*			The referenced sampler must not be moved or destroyed for the lifetime of the combined_image_sampler.
*/
template <image_type type>
auto make_combined_image_sampler(image_view<type> &&view,
								 const sampler *samp,
								 image_layout layout) {
	return combined_image_sampler<type>(std::move(view), samp, layout);
}

/**
*	@brief	Constructs a new combined_image_sampler object, referencing an image and taking ownership of a sampler.
*			The referenced image must not be moved or destroyed for the lifetime of the combined_image_sampler.
*/
template <image_type type>
auto make_combined_image_sampler(const image_view<type> *view,
								 sampler &&samp,
								 image_layout layout) {
	return combined_image_sampler<type>(view, std::move(samp), layout);
}

/**
*	@brief	Constructs a new combined_image_sampler object, taking ownership of an image and sampler.
*/
template <image_type type>
auto make_combined_image_sampler(image_view<type> &&view,
								 sampler &&samp,
								 image_layout layout) {
	return combined_image_sampler<type>(std::move(view), std::move(samp), layout);
}

/**
*	@brief	Constructs a new combined_image_sampler object, referencing a texture.
*			The referenced texture must not be moved or destroyed for the lifetime of the combined_image_sampler.
*/
template <image_type type>
auto make_combined_image_sampler(const texture<type> &tex,
								 const sampler *samp) {
	return combined_image_sampler<type>(tex.get_image_view(), samp, tex.get_layout());
}

/**
*	@brief	Constructs a new combined_image_sampler object, referencing a texture.
*			The referenced texture must not be moved or destroyed for the lifetime of the combined_image_sampler.
*/
template <image_type type>
auto make_combined_image_sampler(const texture<type> &tex,
								 sampler &&samp) {
	return combined_image_sampler<type>(tex.get_image_view(), std::move(samp), tex.get_layout());
}

}
}
