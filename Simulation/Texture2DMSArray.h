// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "Texture.h"

namespace StE {
namespace LLR {

class Texture2DMSArray : public texture_multisampled<llr_resource_type::LLRTexture2DMSArray> {
public:
	Texture2DMSArray(Texture2DMSArray &&m) = default;
	Texture2DMSArray& operator=(Texture2DMSArray &&m) = default;

	Texture2DMSArray(gli::format format, const size_type &size, int samples) : texture_multisampled(format, size, samples) {}
};

}
}
