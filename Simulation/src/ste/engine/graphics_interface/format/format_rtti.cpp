
#include <stdafx.hpp>
#include <format_rtti.hpp>

using namespace ste::gl;

std::unordered_map<format, format_rtti> _internal::format_rtti_database::database =
{
	{
		format::r4g4_unorm_pack8,
		format_rtti {
			2,
			1,
			false,
			false,
			false
		}
	},
	{
		format::r4g4b4a4_unorm_pack16,
		format_rtti {
			4,
			2,
			false,
			false,
			false
		}
	},
	{
		format::b4g4r4a4_unorm_pack16,
		format_rtti {
			4,
			2,
			false,
			false,
			false
		}
	},
	{
		format::r5g6b5_unorm_pack16,
		format_rtti {
			3,
			2,
			false,
			false,
			false
		}
	},
	{
		format::b5g6r5_unorm_pack16,
		format_rtti {
			3,
			2,
			false,
			false,
			false
		}
	},
	{
		format::r5g5b5a1_unorm_pack16,
		format_rtti {
			4,
			2,
			false,
			false,
			false
		}
	},
	{
		format::b5g5r5a1_unorm_pack16,
		format_rtti {
			4,
			2,
			false,
			false,
			false
		}
	},
	{
		format::a1r5g5b5_unorm_pack16,
		format_rtti {
			4,
			2,
			false,
			false,
			false
		}
	},
	{
		format::r8_unorm,
		format_rtti {
			1,
			1,
			false,
			false,
			false
		}
	},
	{
		format::r8_snorm,
		format_rtti {
			1,
			1,
			false,
			false,
			true
		}
	},
	{
		format::r8_uscaled,
		format_rtti {
			1,
			1,
			false,
			false,
			false
		}
	},
	{
		format::r8_sscaled,
		format_rtti {
			1,
			1,
			false,
			false,
			true
		}
	},
	{ 
		format::r8_uint,
		format_rtti {
			1,
			1,
			false,
			false,
			false
		}
	},
	{ 
		format::r8_sint,
		format_rtti {
			1,
			1,
			false,
			false,
			true
		}
	},
	{
		format::r8_srgb,
		format_rtti {
			1,
			1,
			false,
			false,
			false
		}
	},
	{
		format::r8g8_unorm,
		format_rtti {
			2,
			2,
			false,
			false,
			false
		}
	},
	{
		format::r8g8_snorm,
		format_rtti {
			2,
			2,
			false,
			false,
			true
		}
	},
	{
		format::r8g8_uscaled,
		format_rtti {
			2,
			2,
			false,
			false,
			false
		}
	},
	{
		format::r8g8_sscaled,
		format_rtti {
			2,
			2,
			false,
			false,
			true
		}
	},
	{
		format::r8g8_uint,
		format_rtti {
			2,
			2,
			false,
			false,
			false
		}
	},
	{
		format::r8g8_sint,
		format_rtti {
			2,
			2,
			false,
			false,
			true
		}
	},
	{
		format::r8g8_srgb,
		format_rtti {
			2,
			2,
			false,
			false,
			false
		}
	},
	{
		format::r8g8b8_unorm,
		format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		format::r8g8b8_snorm,
		format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		format::r8g8b8_uscaled,
		format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		format::r8g8b8_sscaled,
		format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		format::r8g8b8_uint,
		format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		format::r8g8b8_sint,
		format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		format::r8g8b8_srgb,
		format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		format::b8g8r8_unorm,
		format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		format::b8g8r8_snorm,
		format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		format::b8g8r8_uscaled,
		format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		format::b8g8r8_sscaled,
		format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		format::b8g8r8_uint,
		format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		format::b8g8r8_sint,
		format_rtti {
			3,
			3,
			false,
			false,
			true
		}
	},
	{
		format::b8g8r8_srgb,
		format_rtti {
			3,
			3,
			false,
			false,
			false
		}
	},
	{
		format::r8g8b8a8_unorm,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::r8g8b8a8_snorm,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::r8g8b8a8_uscaled,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::r8g8b8a8_sscaled,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::r8g8b8a8_uint,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::r8g8b8a8_sint,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::r8g8b8a8_srgb,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::b8g8r8a8_unorm,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::b8g8r8a8_snorm,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::b8g8r8a8_uscaled,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::b8g8r8a8_sscaled,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::b8g8r8a8_uint,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::b8g8r8a8_sint,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::b8g8r8a8_srgb,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a8b8g8r8_unorm_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a8b8g8r8_snorm_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::a8b8g8r8_uscaled_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a8b8g8r8_sscaled_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::a8b8g8r8_uint_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a8b8g8r8_sint_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::a8b8g8r8_srgb_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a2r10g10b10_unorm_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a2r10g10b10_snorm_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::a2r10g10b10_uscaled_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a2r10g10b10_sscaled_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::a2r10g10b10_uint_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a2r10g10b10_sint_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::a2b10g10r10_unorm_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a2b10g10r10_snorm_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::a2b10g10r10_uscaled_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a2b10g10r10_sscaled_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::a2b10g10r10_uint_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			false
		}
	},
	{
		format::a2b10g10r10_sint_pack32,
		format_rtti {
			4,
			4,
			false,
			false,
			true
		}
	},
	{
		format::r16_unorm,
		format_rtti {
			1,
			2,
			false,
			false,
			false
		}
	},
	{
		format::r16_snorm,
		format_rtti {
			1,
			2,
			false,
			false,
			true
		}
	},
	{
		format::r16_uscaled,
		format_rtti {
			1,
			2,
			false,
			false,
			false
		}
	},
	{
		format::r16_sscaled,
		format_rtti {
			1,
			2,
			false,
			false,
			true
		}
	},
	{
		format::r16_uint,
		format_rtti {
			1,
			2,
			false,
			false,
			false
		}
	},
	{
		format::r16_sint,
		format_rtti {
			1,
			2,
			false,
			false,
			true
		}
	},
	{
		format::r16_sfloat,
		format_rtti {
			1,
			2,
			false,
			true,
			true
		}
	},
	{
		format::r16g16_unorm,
		format_rtti {
			2,
			4,
			false,
			false,
			false
		}
	},
	{
		format::r16g16_snorm,
		format_rtti {
			2,
			4,
			false,
			false,
			true
		}
	},
	{
		format::r16g16_uscaled,
		format_rtti {
			2,
			4,
			false,
			false,
			false
		}
	},
	{
		format::r16g16_sscaled,
		format_rtti {
			2,
			4,
			false,
			false,
			true
		}
	},
	{
		format::r16g16_uint,
		format_rtti {
			2,
			4,
			false,
			false,
			false
		}
	},
	{
		format::r16g16_sint,
		format_rtti {
			2,
			4,
			false,
			false,
			true
		}
	},
	{
		format::r16g16_sfloat,
		format_rtti {
			2,
			4,
			false,
			true,
			true
		}
	},
	{
		format::r16g16b16_unorm,
		format_rtti {
			3,
			6,
			false,
			false,
			false
		}
	},
	{
		format::r16g16b16_snorm,
		format_rtti {
			3,
			6,
			false,
			false,
			true
		}
	},
	{
		format::r16g16b16_uscaled,
		format_rtti {
			3,
			6,
			false,
			false,
			false
		}
	},
	{
		format::r16g16b16_sscaled,
		format_rtti {
			3,
			6,
			false,
			false,
			false
		}
	},
	{
		format::r16g16b16_uint,
		format_rtti {
			3,
			6,
			false,
			false,
			false
		}
	},
	{
		format::r16g16b16_sint,
		format_rtti {
			3,
			6,
			false,
			false,
			true
		}
	},
	{
		format::r16g16b16_sfloat,
		format_rtti {
			3,
			6,
			false,
			true,
			true
		}
	},
	{
		format::r16g16b16a16_unorm,
		format_rtti {
			4,
			8,
			false,
			false,
			false
		}
	},
	{
		format::r16g16b16a16_snorm,
		format_rtti {
			4,
			8,
			false,
			false,
			true
		}
	},
	{
		format::r16g16b16a16_uscaled,
		format_rtti {
			4,
			8,
			false,
			false,
			false
		}
	},
	{
		format::r16g16b16a16_sscaled,
		format_rtti {
			4,
			8,
			false,
			false,
			true
		}
	},
	{
		format::r16g16b16a16_uint,
		format_rtti {
			4,
			8,
			false,
			false,
			false
		}
	},
	{
		format::r16g16b16a16_sint,
		format_rtti {
			4,
			8,
			false,
			false,
			true
		}
	},
	{
		format::r16g16b16a16_sfloat,
		format_rtti {
			4,
			8,
			false,
			true,
			true
		}
	},
	{
		format::r32_uint,
		format_rtti {
			1,
			4,
			false,
			false,
			false
		}
	},
	{
		format::r32_sint,
		format_rtti {
			1,
			4,
			false,
			false,
			true
		}
	},
	{
		format::r32_sfloat,
		format_rtti {
			1,
			4,
			false,
			true,
			true
		}
	},
	{
		format::r32g32_uint,
		format_rtti {
			2,
			8,
			false,
			false,
			false
		}
	},
	{
		format::r32g32_sint,
		format_rtti {
			2,
			8,
			false,
			false,
			true
		}
	},
	{
		format::r32g32_sfloat,
		format_rtti {
			2,
			8,
			false,
			true,
			true
		}
	},
	{
		format::r32g32b32_uint,
		format_rtti {
			3,
			12,
			false,
			false,
			false
		}
	},
	{
		format::r32g32b32_sint,
		format_rtti {
			3,
			12,
			false,
			false,
			true
		}
	},
	{
		format::r32g32b32_sfloat,
		format_rtti {
			3,
			12,
			false,
			true,
			true
		}
	},
	{
		format::r32g32b32a32_uint,
		format_rtti {
			4,
			16,
			false,
			false,
			false
		}
	},
	{
		format::r32g32b32a32_sint,
		format_rtti {
			4,
			16,
			false,
			false,
			true
		}
	},
	{
		format::r32g32b32a32_sfloat,
		format_rtti {
			4,
			16,
			false,
			true,
			true
		}
	},
	{
		format::r64_uint,
		format_rtti {
			1,
			8,
			false,
			false,
			false
		}
	},
	{
		format::r64_sint,
		format_rtti {
			1,
			8,
			false,
			false,
			true
		}
	},
	{
		format::r64_sfloat,
		format_rtti {
			1,
			8,
			false,
			true,
			true
		}
	},
	{
		format::r64g64_uint,
		format_rtti {
			2,
			16,
			false,
			false,
			false
		}
	},
	{
		format::r64g64_sint,
		format_rtti {
			2,
			16,
			false,
			false,
			true
		}
	},
	{
		format::r64g64_sfloat,
		format_rtti {
			2,
			16,
			false,
			true,
			true
		}
	},
	{
		format::r64g64b64_uint,
		format_rtti {
			3,
			24,
			false,
			false,
			false
		}
	},
	{
		format::r64g64b64_sint,
		format_rtti {
			3,
			24,
			false,
			false,
			true
		}
	},
	{
		format::r64g64b64_sfloat,
		format_rtti {
			3,
			24,
			false,
			true,
			true
		}
	},
	{
		format::r64g64b64a64_uint,
		format_rtti {
			4,
			32,
			false,
			false,
			false
		}
	},
	{
		format::r64g64b64a64_sint,
		format_rtti {
			4,
			32,
			false,
			false,
			true
		}
	},
	{
		format::r64g64b64a64_sfloat,
		format_rtti {
			4,
			32,
			false,
			true,
			true
		}
	},
	{
		format::b10g11r11_ufloat_pack32,
		format_rtti {
			3,
			4,
			false,
			true,
			false
		}
	},
	{
		format::e5b9g9r9_ufloat_pack32,
		format_rtti {
			3,
			4,
			false,
			true,
			false
		}
	},
	{
		format::d16_unorm,
		format_rtti {
			1,
			2,
			true,
			false,
			false
		}
	},
	{
		format::x8_d24_unorm_pack32,
		format_rtti {
			1,
			4,
			true,
			false,
			false
		}
	},
	{
		format::d32_sfloat,
		format_rtti {
			1,
			4,
			true,
			true,
			true
		}
	}
};
