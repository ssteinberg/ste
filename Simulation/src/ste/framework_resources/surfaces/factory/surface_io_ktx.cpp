
#include <stdafx.hpp>
#include <surface_io.hpp>
#include <surface_io_helper.hpp>

#include <format_rtti.hpp>

#include <log.hpp>
#include <attributed_string.hpp>
#include <attrib.hpp>

#include <lib/unique_ptr.hpp>

using namespace ste;
using namespace ste::text;
using namespace ste::resource;
using namespace surface_io_helper;

namespace ste::resource::_detail {

inline gl::image_type ktx_image_type(const ktx_header10& header) {
	if (header.NumberOfFaces > 1)
		return header.NumberOfArrayElements > 1 ? gl::image_type::image_cubemap_array : gl::image_type::image_cubemap;
	if (header.NumberOfArrayElements > 1)
		return header.PixelHeight > 0 ? gl::image_type::image_2d_array : gl::image_type::image_1d_array;
	if (header.PixelHeight == 0)
		return gl::image_type::image_1d;
	if (header.PixelDepth > 0)
		return gl::image_type::image_3d;
	return gl::image_type::image_2d;
}

template <int dimensions>
opaque_surface<dimensions> load_ktx(const std::experimental::filesystem::path &path) {
	using surface_t = opaque_surface<dimensions>;
	using extent_type = gl::image_extent_type_t<dimensions>;

	// Read file
	std::ifstream fs(path.string(), std::ios::in | std::ios::binary);
	if (!fs) {
		using namespace attributes;
		ste_log_error() << text::attributed_string("Can't open KTX ") + i(lib::to_string(path.string())) + ": " + std::strerror(errno) << std::endl;
		throw resource_io_error("Could not open file");
	}

	auto content = lib::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
	fs.close();

	if (content.size() <= sizeof(magic_ktx10) + sizeof(ktx_header10)) {
		ste_log_error() << "Can't open KTX or file empty: " << path << std::endl;
		throw resource_io_error("Reading file failed");
	}

	// Verify magic
	if (memcmp(content.data(), magic_ktx10, sizeof(magic_ktx10)) != 0) {
		ste_log_error() << "Unsupported KTX10 file: " << path << std::endl;
		throw resource_io_error("Unsupported KTX10 file");
	}

	// Read header
	const ktx_header10 &header = *reinterpret_cast<const ktx_header10 *>(content.data() + sizeof(magic_ktx10));
	const auto image_type = ktx_image_type(header);

	const auto data = content.data();
	auto offset = sizeof(magic_ktx10) + sizeof(ktx_header10) + header.BytesOfKeyValueData;

	// Query format
	gl::format format = gl::format::undefined;

	if (header.GLInternalFormat == INTERNAL_RG4_EXT && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_UINT8_RG4_REV_GTC)					format = gl::format::r4g4_unorm_pack8;
	else if (header.GLInternalFormat == INTERNAL_RGBA4 && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_UINT16_RGBA4_REV)				format = gl::format::r4g4b4a4_unorm_pack16;
	else if (header.GLInternalFormat == INTERNAL_RGBA4 && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_UINT16_RGBA4)					format = gl::format::b4g4r4a4_unorm_pack16;
	else if (header.GLInternalFormat == INTERNAL_R5G6B5 && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_UINT16_R5G6B5_REV)				format = gl::format::r5g6b5_unorm_pack16;
	else if (header.GLInternalFormat == INTERNAL_R5G6B5 && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_UINT16_R5G6B5)					format = gl::format::b5g6r5_unorm_pack16;
	else if (header.GLInternalFormat == INTERNAL_RGB5A1 && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_UINT16_RGB5A1_REV)				format = gl::format::r5g5b5a1_unorm_pack16;
	else if (header.GLInternalFormat == INTERNAL_RGB5A1 && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_UINT16_RGB5A1)					format = gl::format::b5g5r5a1_unorm_pack16;
	else if (header.GLInternalFormat == INTERNAL_RGB5A1 && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_UINT16_A1RGB5_GTC)				format = gl::format::a1r5g5b5_unorm_pack16;

	else if (header.GLInternalFormat == INTERNAL_R8_UNORM && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_U8)							format = gl::format::r8_unorm;
	else if (header.GLInternalFormat == INTERNAL_R8_SNORM && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_I8)							format = gl::format::r8_snorm;
	else if (header.GLInternalFormat == INTERNAL_R8_USCALED_GTC && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_U8)						format = gl::format::r8_uscaled;
	else if (header.GLInternalFormat == INTERNAL_R8_SSCALED_GTC && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_I8)						format = gl::format::r8_sscaled;
	else if (header.GLInternalFormat == INTERNAL_R8U && header.GLFormat == EXTERNAL_RED_INTEGER && header.GLType == TYPE_U8)						format = gl::format::r8_uint;
	else if (header.GLInternalFormat == INTERNAL_R8I && header.GLFormat == EXTERNAL_RED_INTEGER && header.GLType == TYPE_I8)						format = gl::format::r8_sint;
	else if (header.GLInternalFormat == INTERNAL_SR8 && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_U8)								format = gl::format::r8_srgb;

	else if (header.GLInternalFormat == INTERNAL_RG8_UNORM && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_U8)							format = gl::format::r8g8_unorm;
	else if (header.GLInternalFormat == INTERNAL_RG8_SNORM && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_I8)							format = gl::format::r8g8_snorm;
	else if (header.GLInternalFormat == INTERNAL_RG8_USCALED_GTC && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_U8)						format = gl::format::r8g8_uscaled;
	else if (header.GLInternalFormat == INTERNAL_RG8_SSCALED_GTC && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_I8)						format = gl::format::r8g8_sscaled;
	else if (header.GLInternalFormat == INTERNAL_RG8U && header.GLFormat == EXTERNAL_RG_INTEGER && header.GLType == TYPE_U8)						format = gl::format::r8g8_uint;
	else if (header.GLInternalFormat == INTERNAL_RG8I && header.GLFormat == EXTERNAL_RG_INTEGER && header.GLType == TYPE_I8)						format = gl::format::r8g8_sint;
	else if (header.GLInternalFormat == INTERNAL_SRG8 && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_U8)								format = gl::format::r8g8_srgb;

	else if (header.GLInternalFormat == INTERNAL_RGB_UNORM && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_U8)							format = gl::format::r8g8b8_unorm;
	else if (header.GLInternalFormat == INTERNAL_RGB8_UNORM && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_U8)							format = gl::format::r8g8b8_unorm;
	else if (header.GLInternalFormat == INTERNAL_RGB8_SNORM && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_I8)							format = gl::format::r8g8b8_snorm;
	else if (header.GLInternalFormat == INTERNAL_RGB8_USCALED_GTC && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_U8)					format = gl::format::r8g8b8_uscaled;
	else if (header.GLInternalFormat == INTERNAL_RGB8_SSCALED_GTC && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_I8)					format = gl::format::r8g8b8_sscaled;
	else if (header.GLInternalFormat == INTERNAL_RGB8U && header.GLFormat == EXTERNAL_RGB_INTEGER && header.GLType == TYPE_U8)						format = gl::format::r8g8b8_uint;
	else if (header.GLInternalFormat == INTERNAL_RGB8I && header.GLFormat == EXTERNAL_RGB_INTEGER && header.GLType == TYPE_I8)						format = gl::format::r8g8b8_sint;

	else if (header.GLInternalFormat == INTERNAL_RGBA_UNORM && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_U8)						format = gl::format::r8g8b8a8_unorm;
	else if (header.GLInternalFormat == INTERNAL_RGBA8_UNORM && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_U8)						format = gl::format::r8g8b8a8_unorm;
	else if (header.GLInternalFormat == INTERNAL_RGBA8_SNORM && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_I8)						format = gl::format::r8g8b8a8_snorm;
	else if (header.GLInternalFormat == INTERNAL_RGBA8_USCALED_GTC && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_U8)					format = gl::format::r8g8b8a8_uscaled;
	else if (header.GLInternalFormat == INTERNAL_RGBA8_SSCALED_GTC && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_I8)					format = gl::format::r8g8b8a8_sscaled;
	else if (header.GLInternalFormat == INTERNAL_RGBA8U && header.GLFormat == EXTERNAL_RGBA_INTEGER && header.GLType == TYPE_U8)					format = gl::format::r8g8b8a8_uint;
	else if (header.GLInternalFormat == INTERNAL_RGBA8I && header.GLFormat == EXTERNAL_RGBA_INTEGER && header.GLType == TYPE_I8)					format = gl::format::r8g8b8a8_sint;

	else if (header.GLInternalFormat == INTERNAL_BGR_UNORM && header.GLFormat == EXTERNAL_BGR && header.GLType == TYPE_U8)							format = gl::format::b8g8r8_unorm;
	else if (header.GLInternalFormat == INTERNAL_BGRA_UNORM && header.GLFormat == EXTERNAL_BGRA && header.GLType == TYPE_U8)						format = gl::format::b8g8r8a8_unorm;
	else if (header.GLInternalFormat == INTERNAL_RGBA8_UNORM && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_UINT32_RGBA8_REV)			format = gl::format::a8b8g8r8_unorm_pack32;
	else if (header.GLInternalFormat == INTERNAL_RGBA8_SNORM && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_UINT32_RGBA8_REV)			format = gl::format::a8b8g8r8_snorm_pack32;
	else if (header.GLInternalFormat == INTERNAL_RGBA8_USCALED_GTC && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_UINT32_RGBA8_REV)	format = gl::format::a8b8g8r8_uscaled_pack32;
	else if (header.GLInternalFormat == INTERNAL_RGBA8_SSCALED_GTC && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_UINT32_RGBA8_REV)	format = gl::format::a8b8g8r8_sscaled_pack32;
	else if (header.GLInternalFormat == INTERNAL_RGBA8U && header.GLFormat == EXTERNAL_RGBA_INTEGER && header.GLType == TYPE_UINT32_RGBA8_REV)		format = gl::format::a8b8g8r8_uint_pack32;
	else if (header.GLInternalFormat == INTERNAL_RGBA8I && header.GLFormat == EXTERNAL_RGBA_INTEGER && header.GLType == TYPE_UINT32_RGBA8_REV)		format = gl::format::a8b8g8r8_sint_pack32;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8 && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_UINT32_RGBA8_REV)		format = gl::format::a8b8g8r8_srgb_pack32;

	if (header.GLInternalFormat == INTERNAL_R16_UNORM && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_U16)								format = gl::format::r16_unorm;
	else if (header.GLInternalFormat == INTERNAL_R16_SNORM && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_I16)							format = gl::format::r16_snorm;
	else if (header.GLInternalFormat == INTERNAL_R16_USCALED_GTC && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_U16)					format = gl::format::r16_uscaled;
	else if (header.GLInternalFormat == INTERNAL_R16_SSCALED_GTC && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_I16)					format = gl::format::r16_sscaled;
	else if (header.GLInternalFormat == INTERNAL_R16U && header.GLFormat == EXTERNAL_RED_INTEGER && header.GLType == TYPE_U16)						format = gl::format::r16_uint;
	else if (header.GLInternalFormat == INTERNAL_R16I && header.GLFormat == EXTERNAL_RED_INTEGER && header.GLType == TYPE_I16)						format = gl::format::r16_sint;
	else if (header.GLInternalFormat == INTERNAL_R16F && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_F16)								format = gl::format::r16_sfloat;

	else if (header.GLInternalFormat == INTERNAL_RG16_UNORM && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_U16)							format = gl::format::r16g16_unorm;
	else if (header.GLInternalFormat == INTERNAL_RG16_SNORM && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_I16)							format = gl::format::r16g16_snorm;
	else if (header.GLInternalFormat == INTERNAL_RG16_USCALED_GTC && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_U16)					format = gl::format::r16g16_uscaled;
	else if (header.GLInternalFormat == INTERNAL_RG16_SSCALED_GTC && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_I16)					format = gl::format::r16g16_sscaled;
	else if (header.GLInternalFormat == INTERNAL_RG16U && header.GLFormat == EXTERNAL_RG_INTEGER && header.GLType == TYPE_U16)						format = gl::format::r16g16_uint;
	else if (header.GLInternalFormat == INTERNAL_RG16I && header.GLFormat == EXTERNAL_RG_INTEGER && header.GLType == TYPE_I16)						format = gl::format::r16g16_sint;
	else if (header.GLInternalFormat == INTERNAL_RG16F && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_F16)								format = gl::format::r16g16_sfloat;

	else if (header.GLInternalFormat == INTERNAL_RGB16_UNORM && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_U16)						format = gl::format::r16g16b16_unorm;
	else if (header.GLInternalFormat == INTERNAL_RGB16_SNORM && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_I16)						format = gl::format::r16g16b16_snorm;
	else if (header.GLInternalFormat == INTERNAL_RGB16_USCALED_GTC && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_U16)					format = gl::format::r16g16b16_uscaled;
	else if (header.GLInternalFormat == INTERNAL_RGB16_SSCALED_GTC && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_I16)					format = gl::format::r16g16b16_uscaled;
	else if (header.GLInternalFormat == INTERNAL_RGB16U && header.GLFormat == EXTERNAL_RGB_INTEGER && header.GLType == TYPE_U16)					format = gl::format::r16g16b16_uint;
	else if (header.GLInternalFormat == INTERNAL_RGB16I && header.GLFormat == EXTERNAL_RGB_INTEGER && header.GLType == TYPE_I16)					format = gl::format::r16g16b16_sint;
	else if (header.GLInternalFormat == INTERNAL_RGB16F && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_F16)							format = gl::format::r16g16b16_sfloat;

	else if (header.GLInternalFormat == INTERNAL_RGBA16_UNORM && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_U16)						format = gl::format::r16g16b16a16_unorm;
	else if (header.GLInternalFormat == INTERNAL_RGBA16_SNORM && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_I16)						format = gl::format::r16g16b16a16_snorm;
	else if (header.GLInternalFormat == INTERNAL_RGBA16_USCALED_GTC && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_U16)				format = gl::format::r16g16b16a16_uscaled;
	else if (header.GLInternalFormat == INTERNAL_RGBA16_SSCALED_GTC && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_I16)				format = gl::format::r16g16b16a16_sscaled;
	else if (header.GLInternalFormat == INTERNAL_RGBA16U && header.GLFormat == EXTERNAL_RGBA_INTEGER && header.GLType == TYPE_U16)					format = gl::format::r16g16b16a16_uint;
	else if (header.GLInternalFormat == INTERNAL_RGBA16I && header.GLFormat == EXTERNAL_RGBA_INTEGER && header.GLType == TYPE_I16)					format = gl::format::r16g16b16a16_sint;
	else if (header.GLInternalFormat == INTERNAL_RGBA16F && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_F16)							format = gl::format::r16g16b16a16_sfloat;

	else if (header.GLInternalFormat == INTERNAL_R32U && header.GLFormat == EXTERNAL_RED_INTEGER && header.GLType == TYPE_U32)						format = gl::format::r32_uint;
	else if (header.GLInternalFormat == INTERNAL_R32I && header.GLFormat == EXTERNAL_RED_INTEGER && header.GLType == TYPE_I32)						format = gl::format::r32_sint;
	else if (header.GLInternalFormat == INTERNAL_R32F && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_F32)								format = gl::format::r32_sfloat;
	else if (header.GLInternalFormat == INTERNAL_RG32U && header.GLFormat == EXTERNAL_RG_INTEGER && header.GLType == TYPE_U32)						format = gl::format::r32g32_uint;
	else if (header.GLInternalFormat == INTERNAL_RG32I && header.GLFormat == EXTERNAL_RG_INTEGER && header.GLType == TYPE_I32)						format = gl::format::r32g32_sint;
	else if (header.GLInternalFormat == INTERNAL_RG32F && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_F32)								format = gl::format::r32g32_sfloat;
	else if (header.GLInternalFormat == INTERNAL_RGB32U && header.GLFormat == EXTERNAL_RGB_INTEGER && header.GLType == TYPE_U32)					format = gl::format::r32g32b32_uint;
	else if (header.GLInternalFormat == INTERNAL_RGB32I && header.GLFormat == EXTERNAL_RGB_INTEGER && header.GLType == TYPE_I32)					format = gl::format::r32g32b32_sint;
	else if (header.GLInternalFormat == INTERNAL_RGB32F && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_F32)							format = gl::format::r32g32b32_sfloat;
	else if (header.GLInternalFormat == INTERNAL_RGBA32U && header.GLFormat == EXTERNAL_RGBA_INTEGER && header.GLType == TYPE_U32)					format = gl::format::r32g32b32a32_uint;
	else if (header.GLInternalFormat == INTERNAL_RGBA32I && header.GLFormat == EXTERNAL_RGBA_INTEGER && header.GLType == TYPE_I32)					format = gl::format::r32g32b32a32_sint;
	else if (header.GLInternalFormat == INTERNAL_RGBA32F && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_F32)							format = gl::format::r32g32b32a32_sfloat;
	
	else if (header.GLInternalFormat == INTERNAL_R64F_EXT && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_U64)							format = gl::format::r64_uint;
	else if (header.GLInternalFormat == INTERNAL_R64F_EXT && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_I64)							format = gl::format::r64_sint;
	else if (header.GLInternalFormat == INTERNAL_R64F_EXT && header.GLFormat == EXTERNAL_RED && header.GLType == TYPE_F64)							format = gl::format::r64_sfloat;
	else if (header.GLInternalFormat == INTERNAL_RG64F_EXT && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_U64)							format = gl::format::r64g64_uint;
	else if (header.GLInternalFormat == INTERNAL_RG64F_EXT && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_I64)							format = gl::format::r64g64_sint;
	else if (header.GLInternalFormat == INTERNAL_RG64F_EXT && header.GLFormat == EXTERNAL_RG && header.GLType == TYPE_F64)							format = gl::format::r64g64_sfloat;
	else if (header.GLInternalFormat == INTERNAL_RGB64F_EXT && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_U64)						format = gl::format::r64g64b64_uint;
	else if (header.GLInternalFormat == INTERNAL_RGB64F_EXT && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_I64)						format = gl::format::r64g64b64_sint;
	else if (header.GLInternalFormat == INTERNAL_RGB64F_EXT && header.GLFormat == EXTERNAL_RGB && header.GLType == TYPE_F64)						format = gl::format::r64g64b64_sfloat;
	else if (header.GLInternalFormat == INTERNAL_RGBA64F_EXT && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_U64)						format = gl::format::r64g64b64a64_uint;
	else if (header.GLInternalFormat == INTERNAL_RGBA64F_EXT && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_I64)						format = gl::format::r64g64b64a64_sint;
	else if (header.GLInternalFormat == INTERNAL_RGBA64F_EXT && header.GLFormat == EXTERNAL_RGBA && header.GLType == TYPE_F64)						format = gl::format::r64g64b64a64_sfloat;

	if (header.GLInternalFormat == INTERNAL_D16 && header.GLFormat == EXTERNAL_DEPTH && header.GLType == TYPE_NONE)									format = gl::format::d16_unorm;
	else if (header.GLInternalFormat == INTERNAL_D24 && header.GLFormat == EXTERNAL_DEPTH && header.GLType == TYPE_NONE)							format = gl::format::x8_d24_unorm_pack32;
	else if (header.GLInternalFormat == INTERNAL_D32F && header.GLFormat == EXTERNAL_DEPTH && header.GLType == TYPE_NONE)							format = gl::format::d32_sfloat;
	else if (header.GLInternalFormat == INTERNAL_S8_EXT && header.GLFormat == EXTERNAL_STENCIL && header.GLType == TYPE_NONE)						format = gl::format::s8_uint;
	else if (header.GLInternalFormat == INTERNAL_D16S8_EXT && header.GLFormat == EXTERNAL_DEPTH && header.GLType == TYPE_NONE)						format = gl::format::d16_unorm_s8_uint;
	else if (header.GLInternalFormat == INTERNAL_D24S8 && header.GLFormat == EXTERNAL_DEPTH_STENCIL && header.GLType == TYPE_NONE)					format = gl::format::d24_unorm_s8_uint;
	else if (header.GLInternalFormat == INTERNAL_D32FS8X24 && header.GLFormat == EXTERNAL_DEPTH_STENCIL && header.GLType == TYPE_NONE)				format = gl::format::d32_sfloat_s8_uint;

	else if (header.GLInternalFormat == INTERNAL_RGB_DXT1 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)						format = gl::format::bc1_rgb_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB_DXT1 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)						format = gl::format::bc1_rgb_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_DXT1 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)						format = gl::format::bc1_rgba_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB_ALPHA_DXT1 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::bc1_rgba_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_DXT3 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)						format = gl::format::bc3_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB_ALPHA_DXT3 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::bc3_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_DXT5 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)						format = gl::format::bc5_unorm_block;

	else if (header.GLInternalFormat == INTERNAL_SRGB8_ETC2 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)						format = gl::format::etc2_r8g8b8_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ETC2 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)						format = gl::format::etc2_r8g8b8a1_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ETC2_EAC && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::etc2_r8g8b8a1_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_R11_EAC && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)							format = gl::format::eac_r11_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SIGNED_R11_EAC && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::eac_r11_snorm_block;
	else if (header.GLInternalFormat == INTERNAL_RG11_EAC && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)						format = gl::format::eac_r11g11_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SIGNED_RG11_EAC && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::eac_r11g11_snorm_block;

	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_4x4 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_4x4_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_4x4 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_4x4_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_5x4 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_5x4_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_5x4 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_5x4_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_5x5 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_5x5_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_5x5 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_5x5_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_6x5 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_6x5_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_6x5 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_6x5_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_6x6 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_6x6_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_6x6 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_6x6_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_8x5 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_8x5_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_8x5 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_8x5_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_8x6 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_8x6_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_8x6 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_8x6_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_8x8 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_8x8_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_8x8 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_8x8_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_10x5 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_10x5_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_10x5 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_10x5_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_10x6 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_10x6_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_10x6 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_10x6_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_10x8 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_10x8_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_10x8 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_10x8_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_10x10 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_10x10_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_10x10 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_10x10_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_12x10 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_12x10_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_12x10 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_12x10_srgb_block;
	else if (header.GLInternalFormat == INTERNAL_RGBA_ASTC_12x12 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)					format = gl::format::astc_12x12_unorm_block;
	else if (header.GLInternalFormat == INTERNAL_SRGB8_ALPHA8_ASTC_12x12 && header.GLFormat == EXTERNAL_NONE && header.GLType == TYPE_NONE)			format = gl::format::astc_12x12_srgb_block;

	if (format == gl::format::undefined) {
		using namespace attributes;
		ste_log_error() << text::attributed_string("KTX ") + i(lib::to_string(path.string())) + ": Unsupported format" << std::endl;
		throw surface_error("Unsupported KTX format");
	}

	// Create surface
	extent_type extent{ 1 };
	extent.x = header.PixelWidth;
	if constexpr (dimensions > 1)	extent.y = static_cast<std::uint32_t>(header.PixelHeight);
	if constexpr (dimensions > 2)	extent.z = static_cast<std::uint32_t>(header.PixelDepth);
	const std::uint32_t arrays = glm::max<std::uint32_t>(1, header.NumberOfArrayElements);
	const std::uint32_t levels = header.NumberOfMipmapLevels;
	const std::uint32_t layers = image_type == gl::image_type::image_cubemap || image_type == gl::image_type::image_cubemap_array ? 
		arrays * header.NumberOfFaces : arrays;

	if ((image_type == gl::image_type::image_cubemap || image_type == gl::image_type::image_cubemap_array) &&
		header.NumberOfFaces != 6) {
		using namespace attributes;
		ste_log_error() << text::attributed_string("KTX ") + i(lib::to_string(path.string())) + ": Partial cubemaps are unsupported" << std::endl;
		throw surface_error("KTX: Partial cubemaps are unsupported");
	}

	surface_t tex(format,
				  image_type,
				  extent,
				  levels,
				  layers);

	// Read data into surface
	std::size_t read_bytes = 0;
	for (std::uint32_t level = 0; level < levels; ++level) {
		offset = ((offset + 3) / 4) * 4;

		// Read level size
		const auto image_size = *reinterpret_cast<const std::uint32_t*>(data + offset);
		offset += 4;

		assert(tex.bytes(level) == image_size);

		// Read all level data
		std::size_t level_bytes_read = 0;
		for (std::uint32_t layer = 0; layer < layers; ++layer) {
			const auto blocks = tex.blocks(level) / levels;
			const auto bytes = blocks * tex.block_bytes();

			auto image_data = tex.data_at(layer, level);
			std::memcpy(image_data, data + offset + level_bytes_read, bytes);

			level_bytes_read += bytes;
		}

		assert(level_bytes_read == image_size);

		read_bytes += image_size;
		offset += image_size;
	}

	assert(tex.bytes() == read_bytes);

	return tex;
}

}

opaque_surface<1> surface_io::load_ktx_1d(const std::experimental::filesystem::path &file_name) {
	return _detail::load_ktx<1>(file_name);
}

opaque_surface<2> surface_io::load_ktx_2d(const std::experimental::filesystem::path &file_name) {
	return _detail::load_ktx<2>(file_name);
}

opaque_surface<3> surface_io::load_ktx_3d(const std::experimental::filesystem::path &file_name) {
	return _detail::load_ktx<3>(file_name);
}
