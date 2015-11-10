// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {
namespace LLR {

enum class llr_resource_type {
	LLRTexture1D = GL_TEXTURE_1D,
	LLRTexture1DArray = GL_TEXTURE_1D_ARRAY,
	LLRTexture2D = GL_TEXTURE_2D,
	LLRTexture2DMS = GL_TEXTURE_2D_MULTISAMPLE,
	LLRTexture2DArray = GL_TEXTURE_2D_ARRAY,
	LLRTexture2DMSArray = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
	LLRTexture3D = GL_TEXTURE_3D,
	LLRTextureCubeMap = GL_TEXTURE_CUBE_MAP,
	LLRTextureCubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY,
	LLRSamplingDescriptor,
	LLRGLSLProgram,
	LLRGLSLShader,
	LLRImageObject,
	LLRRenderbufferObject = GL_RENDERBUFFER,
	LLRFramebufferObject = GL_FRAMEBUFFER,
	LLRVertexArrayObject,
	LLRVertexBufferObject,
	LLRElementBufferObject,
	LLRPixelBufferObject,
	LLRShaderStorageBufferObject,
	LLRIndirectDrawBufferObject,
	LLRIndirectDispatchBufferObject,
	LLRAtomicCounterBufferObject,
	LLRQueryBufferObject,
	LLRQueryObjectAnySamples = GL_ANY_SAMPLES_PASSED,
	LLRQueryObjectAnySamplesConservative = GL_ANY_SAMPLES_PASSED_CONSERVATIVE,
};

}
}
