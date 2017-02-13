// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

namespace StE {
namespace Core {

enum class core_resource_type {
	Texture1D,
	Texture1DArray,
	Texture2D,
	Texture2DMS,
	Texture2DArray,
	Texture2DMSArray,
	Texture3D,
	TextureCubeMap,
	TextureCubeMapArray,
	SamplingDescriptor,
	GLSLProgram,
	GLSLShader,
	ImageObject,
	RenderbufferObject,
	FramebufferObject,
	VertexArrayObject,
	VertexBufferObject,
	ElementBufferObject,
	PixelBufferObject,
	ShaderStorageBufferObject,
	UniformBufferObject,
	IndirectDrawBufferObject,
	IndirectDispatchBufferObject,
	AtomicCounterBufferObject,
	QueryBufferObject,
	QueryObjectAnySamples,
	QueryObjectAnySamplesConservative,
	QueryObjectTimestamp,
	QueryObjectTimer,
};

}
}
