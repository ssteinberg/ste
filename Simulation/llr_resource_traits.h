// © Shlomi Steinberg, 2015

#pragma once

#include "llr_resource_type.h"
#include "texture_traits.h"

namespace StE {
namespace LLR {

template <llr_resource_type type> struct resource_is_texture { static constexpr bool value = false; };
template <> struct resource_is_texture<llr_resource_type::LLRTexture1D> { static constexpr bool value = true; };
template <> struct resource_is_texture<llr_resource_type::LLRTexture2D> { static constexpr bool value = true; };
template <> struct resource_is_texture<llr_resource_type::LLRTexture3D> { static constexpr bool value = true; };
template <> struct resource_is_texture<llr_resource_type::LLRTexture1DArray> { static constexpr bool value = true; };
template <> struct resource_is_texture<llr_resource_type::LLRTexture2DArray> { static constexpr bool value = true; };
template <> struct resource_is_texture<llr_resource_type::LLRTexture2DMS> { static constexpr bool value = true; };
template <> struct resource_is_texture<llr_resource_type::LLRTexture2DMSArray> { static constexpr bool value = true; };
template <> struct resource_is_texture<llr_resource_type::LLRTextureCubeMap> { static constexpr bool value = true; };
template <> struct resource_is_texture<llr_resource_type::LLRTextureCubeMapArray> { static constexpr bool value = true; };

template <llr_resource_type type> struct resource_is_render_target { static constexpr bool value = resource_is_texture<type>::value; };
template <> struct resource_is_render_target<llr_resource_type::LLRRenderBufferObject> { static constexpr bool value = true; };

}
}
