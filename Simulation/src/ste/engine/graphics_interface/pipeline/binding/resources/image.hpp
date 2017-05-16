//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <image_view.hpp>

#include <image_layout.hpp>

namespace ste {
namespace gl {
namespace pipeline {

/**
*	@brief	An image is a handle to an image view and a layout.
 *			Used in pipeline binding set layouts for sampled-image and storage-image bindings.
*/
class image {
private:
	const image_view_generic *view;
	image_layout layout;

public:
	/**
	*	@brief	Constructs a new combined-image-sampler object, referencing an image view and a layout.
	*/
	image(const image_view_generic &view,
		  image_layout layout = gl::image_layout::shader_read_only_optimal)
		: view(&view),
		layout(layout)
	{}

	image(image&&) = default;
	image &operator=(image&&) = default;
	image(const image&) = default;
	image &operator=(const image&) = default;

	image_layout get_layout() const { return layout; }
	auto& get_image_view() const { return *view; }
};

class storage_image : public image {
public:
	/**
	*	@brief	Constructs a new combined-image-sampler object, referencing an image view and a layout.
	*/
	storage_image(const image_view_generic &view)
		: image(view,
				gl::image_layout::general)
	{}
};

}
}
}
