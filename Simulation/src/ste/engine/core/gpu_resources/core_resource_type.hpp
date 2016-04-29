// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

namespace StE {
namespace Core {

enum class core_resource_type {
	Texture1D = GL_TEXTURE_1D,
	Texture1DArray = GL_TEXTURE_1D_ARRAY,
	Texture2D = GL_TEXTURE_2D,
	Texture2DMS = GL_TEXTURE_2D_MULTISAMPLE,
	Texture2DArray = GL_TEXTURE_2D_ARRAY,
	Texture2DMSArray = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
	Texture3D = GL_TEXTURE_3D,
	TextureCubeMap = GL_TEXTURE_CUBE_MAP,
	TextureCubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY,
	SamplingDescriptor,
	GLSLProgram,
	GLSLShader,
	ImageObject,
	RenderbufferObject = GL_RENDERBUFFER,
	FramebufferObject = GL_FRAMEBUFFER,
	VertexArrayObject,
	VertexBufferObject,
	ElementBufferObject,
	PixelBufferObject,
	ShaderStorageBufferObject,
	IndirectDrawBufferObject,
	IndirectDispatchBufferObject,
	AtomicCounterBufferObject,
	QueryBufferObject,
	QueryObjectAnySamples = GL_ANY_SAMPLES_PASSED,
	QueryObjectAnySamplesConservative = GL_ANY_SAMPLES_PASSED_CONSERVATIVE,
	QueryObjectTimestamp = GL_TIMESTAMP,
	QueryObjectTimer = GL_TIME_ELAPSED,
};

}
}
