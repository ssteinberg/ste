// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "texture_base.hpp"
#include "image.hpp"

namespace StE {
namespace Core {

class texture_2d_ms_array : public texture_multisampled<core_resource_type::Texture2DMSArray> {
private:
	using Base = texture_multisampled<core_resource_type::Texture2DMSArray>;

public:
	texture_2d_ms_array(texture_2d_ms_array &&m) = default;
	texture_2d_ms_array& operator=(texture_2d_ms_array &&m) = default;

	texture_2d_ms_array(gli::format format, const typename Base::size_type &size, int samples) : texture_multisampled(format, size, samples) {}

	image_container<T> operator[](int level) const {
		return image_container<T>(*this, get_image_container_size(), format, image_access_mode::ReadWrite, level, get_image_container_dimensions());
	}
};

}
}
