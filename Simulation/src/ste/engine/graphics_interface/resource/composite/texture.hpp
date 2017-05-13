//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <image_view.hpp>
#include <image_layout.hpp>

#include <composite_resource_component_storage.hpp>

namespace ste {
namespace gl {

class texture_generic {
public:
	virtual ~texture_generic() noexcept {}

	virtual VkImageView get_image_view_handle() const = 0;
	virtual image_layout get_layout() const = 0;
};

/**
*	@brief	A texture is an image view with a predefined layout.
*			Used in pipeline binding set layouts for sampled-image/storage-image bindings.
*/
template <image_type type>
class texture : public texture_generic {
private:
	_internal::composite_resource_component_storage<image_view<type>> view;
	image_layout layout;

public:
	/**
	*	@brief	Constructs a new texture object, referencing an image view.
	*			The referenced image view must not be moved or destroyed for the lifetime of the texture.
	*/
	texture(const image_view<type> *view,
			image_layout layout)
		: view(view),
		layout(layout)
	{}
	/**
	*	@brief	Constructs a new texture object, taking ownership of an image view.
	*/
	texture(image_view<type> &&view,
			image_layout layout)
		: view(std::move(view)),
		layout(layout)
	{}

	texture(texture&&) = default;
	texture &operator=(texture&& o) = default;

	VkImageView get_image_view_handle() const override final { return view.get().get_image_view_handle(); }
	image_layout get_layout() const override final { return layout; }

	auto& get_image_view() const { return view.get(); }
};

}
}
