//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_image_view.hpp>
#include <device_image_base.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class image_view_generic {
public:
	virtual ~image_view_generic() noexcept {}

	virtual VkImageView get_image_view_handle() const = 0;
	virtual image_type get_image_type() const = 0;
	virtual format get_format() const = 0;
};

template <image_type type>
class image_view
	: public image_view_generic,
	public allow_type_decay<image_view<type>, vk::vk_image_view<type>>
{
private:
	vk::vk_image_view<type> view;

public:
	template <typename... Ts>
	image_view(Ts&&... ts)
		: view(std::forward<Ts>(ts)...)
	{}

	virtual ~image_view() noexcept {}
	VkImageView get_image_view_handle() const override final {
		return view;
	}
	image_type get_image_type() const override final {
		return type;
	}
	format get_format() const override final {
		return static_cast<format>(view.get_format());
	}

	auto& get() const { return view; }

	image_view(image_view&&) = default;
	image_view &operator=(image_view&&) = default;
};

}
}
