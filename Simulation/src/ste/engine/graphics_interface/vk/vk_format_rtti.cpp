
#include <stdafx.hpp>
#include <vk_format_rtti.hpp>

using namespace StE::GL;

std::unordered_map<VkFormat, vk_format_rtti> vk_format_rtti_database::database = 
{
	{
		VK_FORMAT_R4G4_UNORM_PACK8,
		vk_format_rtti {
			2,
			1,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R4G4B4A4_UNORM_PACK16,
		vk_format_rtti {
			4,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B4G4R4A4_UNORM_PACK16,
		vk_format_rtti {
			4,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R5G6B5_UNORM_PACK16,
		vk_format_rtti {
			3,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B5G6R5_UNORM_PACK16,
		vk_format_rtti {
			3,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R5G5B5A1_UNORM_PACK16,
		vk_format_rtti {
			4,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B5G5R5A1_UNORM_PACK16,
		vk_format_rtti {
			4,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A1R5G5B5_UNORM_PACK16,
		vk_format_rtti {
			4,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8_UNORM,
		vk_format_rtti {
			1,
			1,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8_SNORM,
		vk_format_rtti {
			1,
			1,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8_USCALED,
		vk_format_rtti {
			1,
			1,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8_SSCALED,
		vk_format_rtti {
			1,
			1,
			false,
			false,
			true
		}
	},
	{ 
		VK_FORMAT_R8_UINT,
		vk_format_rtti {
			1,
			1,
			false,
			false,
			false
		}
	},
	{ 
		VK_FORMAT_R8_SINT,
		vk_format_rtti {
			1,
			1,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8_SRGB,
		vk_format_rtti {
			1,
			1,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8_UNORM,
		vk_format_rtti {
			2,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8_SNORM,
		vk_format_rtti {
			2,
			2,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8G8_USCALED,
		vk_format_rtti {
			2,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8_SSCALED,
		vk_format_rtti {
			2,
			2,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8G8_UINT,
		vk_format_rtti {
			2,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8_SINT,
		vk_format_rtti {
			2,
			2,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8G8_SRGB,
		vk_format_rtti {
			2,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8B8_UNORM,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8B8_SNORM,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8G8B8_USCALED,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8B8_SSCALED,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8G8B8_UINT,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8B8_SINT,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8G8B8_SRGB,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B8G8R8_UNORM,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B8G8R8_SNORM,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_B8G8R8_USCALED,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B8G8R8_SSCALED,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_B8G8R8_UINT,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B8G8R8_SINT,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_B8G8R8_SRGB,
		vk_format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8B8A8_UNORM,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8B8A8_SNORM,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8G8B8A8_USCALED,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8B8A8_SSCALED,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8G8B8A8_UINT,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R8G8B8A8_SINT,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R8G8B8A8_SRGB,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B8G8R8A8_UNORM,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B8G8R8A8_SNORM,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_B8G8R8A8_USCALED,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B8G8R8A8_SSCALED,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_B8G8R8A8_UINT,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_B8G8R8A8_SINT,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_B8G8R8A8_SRGB,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A8B8G8R8_UNORM_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A8B8G8R8_SNORM_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_A8B8G8R8_USCALED_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_A8B8G8R8_UINT_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A8B8G8R8_SINT_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_A8B8G8R8_SRGB_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A2R10G10B10_UNORM_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A2R10G10B10_SNORM_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_A2R10G10B10_USCALED_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_A2R10G10B10_UINT_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A2R10G10B10_SINT_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_A2B10G10R10_UNORM_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A2B10G10R10_SNORM_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_A2B10G10R10_USCALED_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_A2B10G10R10_UINT_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_A2B10G10R10_SINT_PACK32,
		vk_format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16_UNORM,
		vk_format_rtti {
			1,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16_SNORM,
		vk_format_rtti {
			1,
			2,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16_USCALED,
		vk_format_rtti {
			1,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16_SSCALED,
		vk_format_rtti {
			1,
			2,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16_UINT,
		vk_format_rtti {
			1,
			2,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16_SINT,
		vk_format_rtti {
			1,
			2,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16_SFLOAT,
		vk_format_rtti {
			1,
			2,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R16G16_UNORM,
		vk_format_rtti {
			2,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16G16_SNORM,
		vk_format_rtti {
			2,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16G16_USCALED,
		vk_format_rtti {
			2,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16G16_SSCALED,
		vk_format_rtti {
			2,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16G16_UINT,
		vk_format_rtti {
			2,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16G16_SINT,
		vk_format_rtti {
			2,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16G16_SFLOAT,
		vk_format_rtti {
			2,
			4,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R16G16B16_UNORM,
		vk_format_rtti {
			3,
			6,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16G16B16_SNORM,
		vk_format_rtti {
			3,
			6,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16G16B16_USCALED,
		vk_format_rtti {
			3,
			6,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16G16B16_SSCALED,
		vk_format_rtti {
			3,
			6,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16G16B16_UINT,
		vk_format_rtti {
			3,
			6,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16G16B16_SINT,
		vk_format_rtti {
			3,
			6,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16G16B16_SFLOAT,
		vk_format_rtti {
			3,
			6,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R16G16B16A16_UNORM,
		vk_format_rtti {
			4,
			8,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16G16B16A16_SNORM,
		vk_format_rtti {
			4,
			8,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16G16B16A16_USCALED,
		vk_format_rtti {
			4,
			8,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16G16B16A16_SSCALED,
		vk_format_rtti {
			4,
			8,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16G16B16A16_UINT,
		vk_format_rtti {
			4,
			8,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R16G16B16A16_SINT,
		vk_format_rtti {
			4,
			8,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R16G16B16A16_SFLOAT,
		vk_format_rtti {
			4,
			8,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R32_UINT,
		vk_format_rtti {
			1,
			4,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R32_SINT,
		vk_format_rtti {
			1,
			4,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R32_SFLOAT,
		vk_format_rtti {
			1,
			4,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R32G32_UINT,
		vk_format_rtti {
			2,
			8,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R32G32_SINT,
		vk_format_rtti {
			2,
			8,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R32G32_SFLOAT,
		vk_format_rtti {
			2,
			8,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R32G32B32_UINT,
		vk_format_rtti {
			3,
			12,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R32G32B32_SINT,
		vk_format_rtti {
			3,
			12,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R32G32B32_SFLOAT,
		vk_format_rtti {
			3,
			12,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R32G32B32A32_UINT,
		vk_format_rtti {
			4,
			16,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R32G32B32A32_SINT,
		vk_format_rtti {
			4,
			16,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R32G32B32A32_SFLOAT,
		vk_format_rtti {
			4,
			16,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R64_UINT,
		vk_format_rtti {
			1,
			8,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R64_SINT,
		vk_format_rtti {
			1,
			8,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R64_SFLOAT,
		vk_format_rtti {
			1,
			8,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R64G64_UINT,
		vk_format_rtti {
			2,
			16,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R64G64_SINT,
		vk_format_rtti {
			2,
			16,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R64G64_SFLOAT,
		vk_format_rtti {
			2,
			16,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R64G64B64_UINT,
		vk_format_rtti {
			3,
			24,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R64G64B64_SINT,
		vk_format_rtti {
			3,
			24,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R64G64B64_SFLOAT,
		vk_format_rtti {
			3,
			24,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_R64G64B64A64_UINT,
		vk_format_rtti {
			4,
			32,
			false,
			false,
			false
		}
	},
	{
		VK_FORMAT_R64G64B64A64_SINT,
		vk_format_rtti {
			4,
			32,
			false,
			false,
			true
		}
	},
	{
		VK_FORMAT_R64G64B64A64_SFLOAT,
		vk_format_rtti {
			4,
			32,
			false,
			true,
			true
		}
	},
	{
		VK_FORMAT_B10G11R11_UFLOAT_PACK32,
		vk_format_rtti {
			3,
			4,
			false,
			true,
			false
		}
	},
	{
		VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
		vk_format_rtti {
			3,
			4,
			false,
			true,
			false
		}
	},
	{
		VK_FORMAT_D16_UNORM,
		vk_format_rtti {
			1,
			2,
			true,
			false,
			false
		}
	},
	{
		VK_FORMAT_X8_D24_UNORM_PACK32,
		vk_format_rtti {
			1,
			4,
			true,
			false,
			false
		}
	},
	{
		VK_FORMAT_D32_SFLOAT,
		vk_format_rtti {
			1,
			4,
			true,
			true,
			true
		}
	}
};
