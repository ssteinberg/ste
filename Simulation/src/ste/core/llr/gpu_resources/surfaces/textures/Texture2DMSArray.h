// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "texture_base.h"
#include "image.h"

namespace StE {
namespace LLR {

class Texture2DMSArray : public texture_multisampled<llr_resource_type::LLRTexture2DMSArray> {
private:
	using Base = texture_multisampled<llr_resource_type::LLRTexture2DMSArray>;

public:
	Texture2DMSArray(Texture2DMSArray &&m) = default;
	Texture2DMSArray& operator=(Texture2DMSArray &&m) = default;

	Texture2DMSArray(gli::format format, const typename Base::size_type &size, int samples) : texture_multisampled(format, size, samples) {}

	const image_container<T> operator[](int level) const {
		return image_container<T>(*this, get_image_container_size(), format, ImageAccessMode::ReadWrite, level, get_image_container_dimensions());
	}
};

}
}
