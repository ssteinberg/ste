// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "Texture.h"

namespace StE {
namespace LLR {

class Texture2DMS : public texture_multisampled<llr_resource_type::LLRTexture2DMS> {
public:
	Texture2DMS(Texture2DMS &&m) = default;
	Texture2DMS& operator=(Texture2DMS &&m) = default;

	Texture2DMS(gli::format format, const size_type &size, int samples) : texture_multisampled(format, size, samples) {}
};

}
}
