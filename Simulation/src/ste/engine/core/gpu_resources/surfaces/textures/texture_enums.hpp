// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

namespace StE {
namespace Core {

enum class ImageAccessMode {
	Read = GL_READ_ONLY,
	Write = GL_WRITE_ONLY,
	ReadWrite = GL_READ_WRITE,
};

enum class TextureWrapMode {
	None = 0,
	ClampToEdge = GL_CLAMP_TO_EDGE,
	ClampToBorder = GL_CLAMP_TO_BORDER,
	Wrap = GL_REPEAT,
	Mirrored = GL_MIRRORED_REPEAT,
};

enum class TextureFiltering {
	None = 0,
	Linear = GL_LINEAR,
	Nearest = GL_NEAREST,
};

enum class TextureCompareMode {
	None = GL_NONE,
	CompareToTextureDepth = GL_COMPARE_REF_TO_TEXTURE,
};

enum class TextureCompareFunc {
	LEqual = GL_LEQUAL,
	GEqual = GL_GEQUAL,
	Less = GL_LESS,
	Greater = GL_GREATER,
	Equal = GL_EQUAL,
	NotEqual = GL_NOTEQUAL,
	Always = GL_ALWAYS,
	Never = GL_NEVER,
};

}
}
