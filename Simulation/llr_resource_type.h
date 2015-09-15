// StE
// © Shlomi Steinberg, 2015

#pragma once

namespace StE {
namespace LLR {

enum class llr_resource_type {
	LLRRenderBufferObject = GL_RENDERBUFFER,
	LLRTexture1D = GL_TEXTURE_1D,
	LLRTexture1DArray = GL_TEXTURE_1D_ARRAY,
	LLRTexture2D = GL_TEXTURE_2D,
	LLRTexture2DMS = GL_TEXTURE_2D_MULTISAMPLE,
	LLRTexture2DArray = GL_TEXTURE_2D_ARRAY,
	LLRTexture2DMSArray = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
	LLRTexture3D = GL_TEXTURE_3D,
	LLRTextureCubeMap = GL_TEXTURE_CUBE_MAP,
	LLRTextureCubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY,
	LLRFrameBufferObject = GL_FRAMEBUFFER,
	LLRVertexArrayObject,
	LLRVertexBufferObject,
	LLRElementBufferObject,
};

}
}
