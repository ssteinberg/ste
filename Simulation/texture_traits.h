// © Shlomi Steinberg, 2015

#pragma once

#include "llr_resource_type.h"

namespace StE {
namespace LLR {

template <llr_resource_type type> struct texture_dimensions {};
template <> struct texture_dimensions<llr_resource_type::LLRTexture1D> { static constexpr int dimensions = 1; };
template <> struct texture_dimensions<llr_resource_type::LLRTexture1DArray> { static constexpr int dimensions = 2; };
template <> struct texture_dimensions<llr_resource_type::LLRTexture2D> { static constexpr int dimensions = 2; };
template <> struct texture_dimensions<llr_resource_type::LLRTexture2DMS> { static constexpr int dimensions = 2; };
template <> struct texture_dimensions<llr_resource_type::LLRTexture2DArray> { static constexpr int dimensions = 3; };
template <> struct texture_dimensions<llr_resource_type::LLRTexture2DMSArray> { static constexpr int dimensions = 3; };
template <> struct texture_dimensions<llr_resource_type::LLRTexture3D> { static constexpr int dimensions = 3; };
template <> struct texture_dimensions<llr_resource_type::LLRTextureCubeMap> { static constexpr int dimensions = 2; };
template <> struct texture_dimensions<llr_resource_type::LLRTextureCubeMapArray> { static constexpr int dimensions = 3; };

template <llr_resource_type type> struct texture_layer_dimensions { static constexpr int dimensions = texture_dimensions<type>::dimensions; };
template <> struct texture_layer_dimensions<llr_resource_type::LLRTexture1DArray> { static constexpr int dimensions = 1; };
template <> struct texture_layer_dimensions<llr_resource_type::LLRTexture2DArray> { static constexpr int dimensions = 2; };
template <> struct texture_layer_dimensions<llr_resource_type::LLRTexture2DMSArray> { static constexpr int dimensions = 2; };
template <> struct texture_layer_dimensions<llr_resource_type::LLRTextureCubeMapArray> { static constexpr int dimensions = 2; };

template <llr_resource_type type> struct texture_is_array : std::false_type {};
template <> struct texture_is_array<llr_resource_type::LLRTexture1DArray> : std::true_type{};
template <> struct texture_is_array<llr_resource_type::LLRTexture2DArray> : std::true_type{};
template <> struct texture_is_array<llr_resource_type::LLRTexture2DMSArray> : std::true_type{};
template <> struct texture_is_array<llr_resource_type::LLRTextureCubeMapArray> : std::true_type{};

template <llr_resource_type type> struct texture_is_multisampled : std::false_type {};
template <> struct texture_is_multisampled<llr_resource_type::LLRTexture2DMS> : std::true_type{};
template <> struct texture_is_multisampled<llr_resource_type::LLRTexture2DMSArray> : std::true_type{};

template <llr_resource_type type> struct texture_has_mipmaps { 
	static constexpr bool value = !texture_is_multisampled<type>::value; 
};

template <llr_resource_type type> struct texture_has_samples_or_levels { static constexpr bool value = texture_is_multisampled<type>::value || texture_has_mipmaps<type>::value; };

template <llr_resource_type type> struct texture_is_pixel_transferable { static constexpr bool value = !texture_is_multisampled<type>::value; };

}
}
