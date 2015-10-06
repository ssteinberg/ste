// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "llr_resource_type.h"

namespace StE {
namespace LLR {

enum class ImageAccessMode {
	Read = GL_READ_ONLY,
	Write = GL_WRITE_ONLY,
	ReadWrite = GL_READ_WRITE,
};

enum class TextureWrapMode {
	None = 0,
	ClampToEdge = GL_CLAMP_TO_EDGE,
	Wrap = GL_REPEAT,
	Mirrored = GL_MIRRORED_REPEAT,
};

enum class TextureFiltering {
	None = 0,
	Linear = GL_LINEAR,
	Nearest = GL_NEAREST,
};

}
}
