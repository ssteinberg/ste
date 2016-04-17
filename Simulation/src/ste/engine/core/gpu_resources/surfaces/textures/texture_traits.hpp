// © Shlomi Steinberg, 2015-2016

#pragma once

#include "core_resource_type.hpp"

#include "texture1D_size_type.hpp"

#include <gli/gli.hpp>

namespace StE {
namespace Core {

template <core_resource_type type> struct texture_dimensions {};
template <> struct texture_dimensions<core_resource_type::Texture1D> { static constexpr int dimensions = 1; };
template <> struct texture_dimensions<core_resource_type::Texture1DArray> { static constexpr int dimensions = 2; };
template <> struct texture_dimensions<core_resource_type::Texture2D> { static constexpr int dimensions = 2; };
template <> struct texture_dimensions<core_resource_type::Texture2DMS> { static constexpr int dimensions = 2; };
template <> struct texture_dimensions<core_resource_type::Texture2DArray> { static constexpr int dimensions = 3; };
template <> struct texture_dimensions<core_resource_type::Texture2DMSArray> { static constexpr int dimensions = 3; };
template <> struct texture_dimensions<core_resource_type::Texture3D> { static constexpr int dimensions = 3; };
template <> struct texture_dimensions<core_resource_type::TextureCubeMap> { static constexpr int dimensions = 2; };
template <> struct texture_dimensions<core_resource_type::TextureCubeMapArray> { static constexpr int dimensions = 3; };

template <core_resource_type type> struct texture_layer_dimensions { static constexpr int dimensions = texture_dimensions<type>::dimensions; };
template <> struct texture_layer_dimensions<core_resource_type::Texture1DArray> { static constexpr int dimensions = 1; };
template <> struct texture_layer_dimensions<core_resource_type::Texture2DArray> { static constexpr int dimensions = 2; };
template <> struct texture_layer_dimensions<core_resource_type::Texture2DMSArray> { static constexpr int dimensions = 2; };
template <> struct texture_layer_dimensions<core_resource_type::TextureCubeMapArray> { static constexpr int dimensions = 2; };

template <core_resource_type type> struct texture_is_array : std::false_type {};
template <> struct texture_is_array<core_resource_type::Texture1DArray> : std::true_type{};
template <> struct texture_is_array<core_resource_type::Texture2DArray> : std::true_type{};
template <> struct texture_is_array<core_resource_type::Texture2DMSArray> : std::true_type{};
template <> struct texture_is_array<core_resource_type::TextureCubeMap> : std::true_type{};
template <> struct texture_is_array<core_resource_type::TextureCubeMapArray> : std::true_type {};

template <core_resource_type type> struct texture_is_multisampled : std::false_type {};
template <> struct texture_is_multisampled<core_resource_type::Texture2DMS> : std::true_type{};
template <> struct texture_is_multisampled<core_resource_type::Texture2DMSArray> : std::true_type{};

template <core_resource_type type> struct texture_has_mipmaps {
	static constexpr bool value = !texture_is_multisampled<type>::value;
};

template <core_resource_type type> struct texture_has_samples_or_levels { static constexpr bool value = texture_is_multisampled<type>::value || texture_has_mipmaps<type>::value; };

template <core_resource_type type> struct texture_is_pixel_transferable { static constexpr bool value = !texture_is_multisampled<type>::value; };

template <int dim> struct texture_size_type {};
template <> struct texture_size_type<1> { using type = texture1D_size_type<int>; };
template <> struct texture_size_type<2> { using type = glm::tvec2<int>; };
template <> struct texture_size_type<3> { using type = glm::tvec3<int>; };

}
}
