// © Shlomi Steinberg, 2015

#pragma once

#include <core_resource_type.hpp>
#include <texture_traits.hpp>

namespace StE {
namespace Core {

template <core_resource_type type> struct resource_is_texture { static constexpr bool value = false; };
template <> struct resource_is_texture<core_resource_type::Texture1D> { static constexpr bool value = true; };
template <> struct resource_is_texture<core_resource_type::Texture2D> { static constexpr bool value = true; };
template <> struct resource_is_texture<core_resource_type::Texture3D> { static constexpr bool value = true; };
template <> struct resource_is_texture<core_resource_type::Texture1DArray> { static constexpr bool value = true; };
template <> struct resource_is_texture<core_resource_type::Texture2DArray> { static constexpr bool value = true; };
template <> struct resource_is_texture<core_resource_type::Texture2DMS> { static constexpr bool value = true; };
template <> struct resource_is_texture<core_resource_type::Texture2DMSArray> { static constexpr bool value = true; };
template <> struct resource_is_texture<core_resource_type::TextureCubeMap> { static constexpr bool value = true; };
template <> struct resource_is_texture<core_resource_type::TextureCubeMapArray> { static constexpr bool value = true; };

template <core_resource_type type> struct resource_is_render_target { static constexpr bool value = resource_is_texture<type>::value; };
template <> struct resource_is_render_target<core_resource_type::RenderbufferObject> { static constexpr bool value = true; };

}
}
