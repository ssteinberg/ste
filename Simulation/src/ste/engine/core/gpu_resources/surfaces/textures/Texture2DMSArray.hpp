// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "texture_base.hpp"
#include "image.hpp"

namespace StE {
namespace Core {

class Texture2DMSArray : public texture_multisampled<core_resource_type::Texture2DMSArray> {
private:
	using Base = texture_multisampled<core_resource_type::Texture2DMSArray>;

public:
	Texture2DMSArray(Texture2DMSArray &&m) = default;
	Texture2DMSArray& operator=(Texture2DMSArray &&m) = default;

	Texture2DMSArray(gli::format format, const typename Base::size_type &size, int samples) : texture_multisampled(format, size, samples) {}

	image_container<T> operator[](int level) const {
		return image_container<T>(*this, get_image_container_size(), format, ImageAccessMode::ReadWrite, level, get_image_container_dimensions());
	}
};

}
}
