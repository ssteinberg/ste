// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "texture_base.hpp"
#include "image.hpp"

namespace StE {
namespace Core {

class texture_2d_ms : public texture_multisampled<core_resource_type::Texture2DMS> {
private:
	using Base = texture_multisampled<core_resource_type::Texture2DMS>;

public:
	texture_2d_ms(texture_2d_ms &&m) = default;
	texture_2d_ms& operator=(texture_2d_ms &&m) = default;

	texture_2d_ms(gli::format format, const typename Base::size_type &size, int samples) : texture_multisampled(format, size, samples) {}

	image<T> operator[](int level) const {
		return image<T>(*this, get_image_container_size(), format, image_access_mode::ReadWrite, level, 0);
	}
};

}
}
