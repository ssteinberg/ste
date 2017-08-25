//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_resource.hpp>
#include <ste_resource_traits.hpp>

#include <device_image.hpp>
#include <image_view.hpp>

#include <image_type_traits.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class texture_generic {
public:
	virtual ~texture_generic() noexcept {}

	virtual VkImageView get_image_view_handle() const = 0;
	virtual const device_image_base& get_image() const = 0;
};

/**
*	@brief	A texture is an image together with an image view.
*/
template <image_type type, int dimensions = image_dimensions_v<type>>
class texture
	: ste_resource_deferred_create_trait, public texture_generic, public allow_type_decay<texture<type, dimensions>, image_view<type>> 
{
private:
	device_image<dimensions> image;
	image_view<type> view;

public:
	/**
	*	@brief	Constructs a new texture object.
	*/
	template <typename... Ts>
	texture(device_image<dimensions> &&image,
			Ts&&... args)
		: image(std::move(image)),
		view(this->image,
			 std::forward<Ts>(args)...)
	{}
	template <class policy, typename... Ts>
	texture(ste_resource<device_image<dimensions>, policy> &&image,
			Ts&&... args)
		: image(std::move(image.get())),
		view(this->image,
			 std::forward<Ts>(args)...)
	{}
	~texture() noexcept {}

	texture(texture&&) = default;
	texture &operator=(texture&& o) = default;

	VkImageView get_image_view_handle() const override final { return view.get_image_view_handle(); }
	const device_image<dimensions>& get_image() const override final { return image; }

	auto& get() const { return view; }
};

}
}
