//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <image_view.hpp>
#include <sampler.hpp>

#include <image_layout.hpp>

namespace ste {
namespace gl {
namespace pipeline {

/**
 *	@brief	A combined_image_sampler is a combination of a sampler and an image view.
 *			Used in pipeline binding set layouts for combined-image-sampler bindings.
 */
class combined_image_sampler {
private:
	const image_view_generic *view;
	const sampler *samp;
	image_layout layout;

public:
	/**
	*	@brief	Constructs a new combined-image-sampler object, referencing an image view and a layout.
	*/
	combined_image_sampler(const image_view_generic &view,
						   const sampler &samp,
						   image_layout layout = gl::image_layout::shader_read_only_optimal)
		: view(&view),
		samp(&samp),
		layout(layout)
	{}

	combined_image_sampler(combined_image_sampler&&) = default;
	combined_image_sampler &operator=(combined_image_sampler&&) = default;
	combined_image_sampler(const combined_image_sampler&) = default;
	combined_image_sampler &operator=(const combined_image_sampler&) = default;

	image_layout get_layout() const { return layout; }
	const sampler& get_sampler() const { return *samp; }
	auto& get_image_view() const { return *view; }
};

}
}
}
