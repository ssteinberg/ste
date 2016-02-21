// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "texture_base.h"
#include "image.h"

namespace StE {
namespace LLR {

class Texture2DMS : public texture_multisampled<llr_resource_type::LLRTexture2DMS> {
private:
	using Base = texture_multisampled<llr_resource_type::LLRTexture2DMS>;

public:
	Texture2DMS(Texture2DMS &&m) = default;
	Texture2DMS& operator=(Texture2DMS &&m) = default;

	Texture2DMS(gli::format format, const typename Base::size_type &size, int samples) : texture_multisampled(format, size, samples) {}

	const image<T> operator[](int level) const {
		return image<T>(*this, get_image_container_size(), format, ImageAccessMode::ReadWrite, level, 0);
	}
};

}
}
