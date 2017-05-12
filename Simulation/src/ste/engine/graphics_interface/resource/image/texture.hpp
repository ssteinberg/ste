//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <image_view.hpp>
#include <sampler.hpp>

#include <image_layout.hpp>

#include <optional.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename T>
struct texture_component_storage {
	optional<T> object;
	const T *pointer{ nullptr };

	texture_component_storage() = delete;
	texture_component_storage(const T *ptr) : pointer(ptr) {}
	texture_component_storage(T &&obj) : object(std::move(obj)) {}

	texture_component_storage(texture_component_storage&&) = default;
	texture_component_storage &operator=(texture_component_storage&&) = default;

	const T &get() const {
		if (object)
			return object.get();
		return *pointer;
	}
};

}

class texture_generic {
public:
	virtual ~texture_generic() noexcept {}

	virtual const sampler& get_sampler() const = 0;
	virtual VkImageView get_image_view_handle() const = 0;
	virtual image_layout get_layout() const = 0;
};

/**
 *	@brief	A texture is a combination of a sampler and an image view.
 *			Used in pipeline binding set layouts for combined-image-sampler bindings.
 */
template <image_type type>
class texture : public texture_generic {
private:
	const _internal::texture_component_storage<image_view<type>> view;
	const _internal::texture_component_storage<sampler> samp;
	image_layout layout;

public:
	texture(const image_view<type> *view,
			const sampler *samp,
			image_layout layout)
		: view(view),
		samp(samp),
		layout(layout)
	{}
	texture(image_view<type> &&view,
			const sampler *samp,
			image_layout layout)
		: view(std::move(view)),
		samp(samp),
		layout(layout)
	{}
	texture(const image_view<type> *view,
			sampler &&samp,
			image_layout layout)
		: view(view),
		samp(std::move(samp)),
		layout(layout)
	{}
	texture(image_view<type> &&view,
			sampler &&samp,
			image_layout layout)
		: view(std::move(view)),
		samp(std::move(samp)),
		layout(layout)
	{}

	texture(texture&& o) noexcept
		: view(std::move(o.view)),
		samp(std::move(o.samp)),
		layout(o.layout)
	{}
	texture &operator=(texture&& o) noexcept {
		view = std::move(o.view);
		samp = std::move(o.samp);
		layout = o.layout;

		return *this;
	}

	const sampler& get_sampler() const override final { return samp.get(); }
	VkImageView get_image_view_handle() const override final { return view.get().get_image_view_handle(); }
	image_layout get_layout() const override final { return layout; }

	auto& get_image_view() const { return view.get(); }
};

/**
 *	@brief	Constructs a new texture object, referencing an image and sampler.
 *			The referenced objects must not be moved or destroyed for the lifetime of the texture.
 */
template <image_type type>
auto make_texture(const image_view<type> *view,
				  const sampler *samp,
				  image_layout layout) {
	return texture<type>(view, samp, layout);
}

/**
*	@brief	Constructs a new texture object, referencing a sampler and taking ownership of an image.
*			The referenced sampler must not be moved or destroyed for the lifetime of the texture.
*/
template <image_type type>
auto make_texture(image_view<type> &&view,
				  const sampler *samp,
				  image_layout layout) {
	return texture<type>(std::move(view), samp, layout);
}

/**
*	@brief	Constructs a new texture object, referencing an image and taking ownership of a sampler.
*			The referenced image must not be moved or destroyed for the lifetime of the texture.
*/
template <image_type type>
auto make_texture(const image_view<type> *view,
				  sampler &&samp,
				  image_layout layout) {
	return texture<type>(view, std::move(samp), layout);
}

/**
*	@brief	Constructs a new texture object, taking ownership of an image and sampler.
*/
template <image_type type>
auto make_texture(image_view<type> &&view,
				  sampler &&samp,
				  image_layout layout) {
	return texture<type>(std::move(view), std::move(samp), layout);
}

}
}
