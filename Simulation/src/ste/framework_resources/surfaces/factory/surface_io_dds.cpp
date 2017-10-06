
#include <stdafx.hpp>
#include <surface_io.hpp>
#include <surface_io_helper.hpp>

#include <format_rtti.hpp>

#include <log.hpp>
#include <attributed_string.hpp>
#include <attrib.hpp>

#include <optional.hpp>

using namespace ste;
using namespace ste::text;
using namespace ste::resource;
using namespace surface_io_helper;

namespace ste::resource::_detail {

inline gl::image_type dds_image_type(const dds_header &header, const dds_header10 &Header10) {
	if (header.CubemapFlags & DDSCAPS2_CUBEMAP)
		return Header10.ArraySize > 1 ? gl::image_type::image_cubemap_array : gl::image_type::image_cubemap;
	if (Header10.ArraySize > 1)
		return header.Flags & DDSD_HEIGHT ? gl::image_type::image_2d_array : gl::image_type::image_1d_array;
	if (Header10.ResourceDimension == D3D10_RESOURCE_DIMENSION_TEXTURE1D)
		return gl::image_type::image_1d;
	if (Header10.ResourceDimension == D3D10_RESOURCE_DIMENSION_TEXTURE3D || header.Flags & DDSD_DEPTH || header.CubemapFlags & DDSCAPS2_VOLUME)
		return gl::image_type::image_3d;
	return gl::image_type::image_2d;
}

inline d3dfmt remap_dds_magic(d3dfmt magic) {
	switch (magic) {
	default:
		return magic;
	case d3dfmt::D3DFMT_BC4U:
		return d3dfmt::D3DFMT_ATI1;
	case d3dfmt::D3DFMT_BC4S:
		return d3dfmt::D3DFMT_AT1N;
	case d3dfmt::D3DFMT_BC5U:
		return d3dfmt::D3DFMT_ATI2;
	case d3dfmt::D3DFMT_BC5S:
		return d3dfmt::D3DFMT_AT2N;
	}
}

template <int dimensions>
opaque_surface<dimensions> load_dds(const std::experimental::filesystem::path &path) {
	using surface_t = opaque_surface<dimensions>;
	using extent_type = gl::image_extent_type_t<dimensions>;

	// Read file
	lib::string content;
	{
		std::ifstream fs(path.string(), std::ios::in | std::ios::binary);
		if (!fs) {
			using namespace attributes;
			ste_log_error() << text::attributed_string("Can't open JPEG ") + i(lib::to_string(path.string())) + ": " + std::strerror(errno) << std::endl;
			throw resource_io_error("Could not open file");
		}

		const std::size_t size = std::experimental::filesystem::file_size(path);
		content.resize(size);
		fs.read(content.data(), size);
	}

	auto header_bytes = sizeof(dds_header) + sizeof(magic_dds);
	auto data = content.data();

	// Validate magic
	if (strncmp(data, reinterpret_cast<const char*>(magic_dds), 4) != 0 ||
		content.size() <= sizeof(dds_header) + sizeof(magic_dds)) {
		using namespace attributes;
		ste_log_error() << text::attributed_string("DDS ") + i(lib::to_string(path.string())) + ": Bad magic or header" << std::endl;
		throw surface_error("Bad magic or header on DDS file");
	}
	data += sizeof(magic_dds);

	// Read header
	const auto header = *reinterpret_cast<const dds_header *>(data);
	data += sizeof(dds_header);

	dds_header10 Header10;
	if ((header.Format.flags & DDPF_FOURCC) && (header.Format.magic == d3dfmt::D3DFMT_DX10)) {
		if (content.size() <= sizeof(dds_header) + sizeof(magic_dds) + sizeof(dds_header10)) {
			using namespace attributes;
			ste_log_error() << text::attributed_string("DDS ") + i(lib::to_string(path.string())) + ": Bad magic or header" << std::endl;
			throw surface_error("Bad magic or header on DDS file");
		}
		std::memcpy(&Header10, data, sizeof(Header10));
		data += sizeof(dds_header10);
		header_bytes += sizeof(dds_header10);
	}

	// Select image type
	const auto image_type = dds_image_type(header, Header10);
	if (gl::image_dimensions_for_type(image_type) != dimensions) {
		using namespace attributes;
		ste_log_error() << text::attributed_string("DDS ") + i(lib::to_string(path.string())) + ": Unexpected dimenions count" << std::endl;
		throw surface_error("Unexpected DDS dimenions count");
	}

	// Select format
	optional<gl::format> format;
	if ((header.Format.flags & (DDPF_RGB | DDPF_ALPHAPIXELS | DDPF_ALPHA | DDPF_YUV | DDPF_LUMINANCE)) && header.Format.bpp != 0) {
		switch (header.Format.bpp) {
		default: {
			using namespace attributes;
			ste_log_error() << text::attributed_string("DDS ") + i(lib::to_string(path.string())) + ": Unsupported format" << std::endl;
			throw surface_error("Unsupported DDS format");
		}
		case 8: {
			if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x000F, 0x00F0, 0x0000, 0x0000 })))
				format = gl::format::r4g4_unorm_pack8;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x000000FF, 0x00000000, 0x00000000, 0x00000000 })) ||
					 glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x00000000, 0x00000000, 0x00000000, 0x000000FF })) ||
					 glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x00FF0000, 0x00000000, 0x00000000, 0x00000000 })))
				format = gl::format::r8_unorm;
			else {
				using namespace attributes;
				ste_log_error() << text::attributed_string("DDS ") + i(lib::to_string(path.string())) + ": Unsupported format" << std::endl;
				throw surface_error("Unsupported DDS format");
			}
			break;
		}
		case 16: {
			if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x000F, 0x00F0, 0x0F00, 0xF000 })))
				format = gl::format::r4g4b4a4_unorm_pack16;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x0F00, 0x00F0, 0x000F, 0xF000 })))
				format = gl::format::b4g4r4a4_unorm_pack16;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x001f, 0x07e0, 0xf800, 0x0000 })))
				format = gl::format::r5g6b5_unorm_pack16;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0xf800, 0x07e0, 0x001f, 0x0000 })))
				format = gl::format::b5g6r5_unorm_pack16;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x001f, 0x03e0, 0x7c00, 0x8000 })))
				format = gl::format::r5g5b5a1_unorm_pack16;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x7c00, 0x03e0, 0x001f, 0x8000 })))
				format = gl::format::b5g5r5a1_unorm_pack16;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x00FF0000, 0x0000FF00, 0x00000000, 0x00000000 })))
				format = gl::format::r8g8_unorm;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x0000FFFF, 0x00000000, 0x00000000, 0x00000000 })) ||
					 glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x0000FFFF, 0x00000000, 0x00000000, 0x00000000 })))
				format = gl::format::r16_unorm;
			else {
				using namespace attributes;
				ste_log_error() << text::attributed_string("DDS ") + i(lib::to_string(path.string())) + ": Unsupported format" << std::endl;
				throw surface_error("Unsupported DDS format");
			}
			break;
		}
		case 24: {
			if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 })))
				format = gl::format::r8g8b8_unorm;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 })))
				format = gl::format::b8g8r8_unorm;
			else {
				using namespace attributes;
				ste_log_error() << text::attributed_string("DDS ") + i(lib::to_string(path.string())) + ": Unsupported format" << std::endl;
				throw surface_error("Unsupported DDS format");
			}
			break;
		}
		case 32: {
			if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 })) ||
				glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 })))
				format = gl::format::b8g8r8a8_unorm;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 })))
				format = gl::format::r8g8b8a8_unorm;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0x0000FFFF, 0xFFFF0000, 0x00000000, 0x00000000 })))
				format = gl::format::r16g16_unorm;
			else if (glm::all(glm::equal(header.Format.Mask, glm::u32vec4{ 0xFFFFFFFF, 0x0000000, 0x0000000, 0x0000000 })))
				format = gl::format::r32_sfloat;
			else {
				using namespace attributes;
				ste_log_error() << text::attributed_string("DDS ") + i(lib::to_string(path.string())) + ": Unsupported format" << std::endl;
				throw surface_error("Unsupported DDS format");
			}
			break;
		}
		}
	}
	else if ((header.Format.flags & DDPF_FOURCC) && (header.Format.magic != d3dfmt::D3DFMT_DX10)) {
		const d3dfmt magic = remap_dds_magic(header.Format.magic);
		switch (magic) {
		case d3dfmt::D3DFMT_R8G8B8:			format = gl::format::b8g8r8_unorm; break;
		case d3dfmt::D3DFMT_A8R8G8B8:		format = gl::format::b8g8r8a8_unorm; break;
		case d3dfmt::D3DFMT_X8R8G8B8:		format = gl::format::b8g8r8a8_unorm; break;
		case d3dfmt::D3DFMT_R5G6B5:			format = gl::format::b5g6r5_unorm_pack16; break;
		case d3dfmt::D3DFMT_X1R5G5B5:		format = gl::format::b5g5r5a1_unorm_pack16; break;
		case d3dfmt::D3DFMT_A1R5G5B5:		format = gl::format::b5g5r5a1_unorm_pack16; break;
		case d3dfmt::D3DFMT_A4R4G4B4:		format = gl::format::b4g4r4a4_unorm_pack16; break;
		case d3dfmt::D3DFMT_X4R4G4B4:		format = gl::format::b4g4r4a4_unorm_pack16; break;
		case d3dfmt::D3DFMT_A8B8G8R8:		format = gl::format::r8g8b8a8_unorm; break;
		case d3dfmt::D3DFMT_X8B8G8R8:		format = gl::format::r8g8b8a8_unorm; break;
		case d3dfmt::D3DFMT_G16R16:			format = gl::format::r16g16_unorm; break;
		case d3dfmt::D3DFMT_A16B16G16R16:	format = gl::format::r16g16b16a16_unorm; break;

		case d3dfmt::D3DFMT_L8:				format = gl::format::r8_unorm; break;

		case d3dfmt::D3DFMT_D16_LOCKABLE:	format = gl::format::d16_unorm; break;
		case d3dfmt::D3DFMT_D16:			format = gl::format::d16_unorm; break;

		case d3dfmt::D3DFMT_D32F_LOCKABLE:	format = gl::format::d32_sfloat; break;

		case d3dfmt::D3DFMT_L16:			format = gl::format::r16_unorm; break;

		case d3dfmt::D3DFMT_R16F:			format = gl::format::r16_sfloat; break;
		case d3dfmt::D3DFMT_G16R16F:		format = gl::format::r16g16_sfloat; break;
		case d3dfmt::D3DFMT_A16B16G16R16F:	format = gl::format::r16g16b16a16_sfloat; break;

		case d3dfmt::D3DFMT_R32F:			format = gl::format::r32_sfloat; break;
		case d3dfmt::D3DFMT_G32R32F:		format = gl::format::r32g32_sfloat; break;
		case d3dfmt::D3DFMT_A32B32G32R32F:	format = gl::format::r32g32b32a32_sfloat; break;
		}
	}
	else if ((header.Format.flags & DDPF_FOURCC) && (header.Format.magic == d3dfmt::D3DFMT_DX10)) {
		switch (Header10.Format) {
		case dxgi_format_dds::DXGI_FORMAT_R32G32B32A32_FLOAT:			format = gl::format::r32g32b32a32_sfloat; break;
		case dxgi_format_dds::DXGI_FORMAT_R32G32B32A32_UINT:			format = gl::format::r32g32b32a32_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R32G32B32A32_SINT:			format = gl::format::r32g32b32a32_sint; break;
		case dxgi_format_dds::DXGI_FORMAT_R32G32B32_FLOAT:				format = gl::format::r32g32b32_sfloat; break;
		case dxgi_format_dds::DXGI_FORMAT_R32G32B32_UINT:				format = gl::format::r32g32b32_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R32G32B32_SINT:				format = gl::format::r32g32b32_sint; break;
		case dxgi_format_dds::DXGI_FORMAT_R16G16B16A16_FLOAT:			format = gl::format::r16g16b16a16_sfloat; break;
		case dxgi_format_dds::DXGI_FORMAT_R16G16B16A16_UNORM:			format = gl::format::r16g16b16a16_unorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R16G16B16A16_UINT:			format = gl::format::r16g16b16a16_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R16G16B16A16_SNORM:			format = gl::format::r16g16b16a16_snorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R16G16B16A16_SINT:			format = gl::format::r16g16b16a16_sint; break;
		case dxgi_format_dds::DXGI_FORMAT_R32G32_FLOAT:					format = gl::format::r32g32_sfloat; break;
		case dxgi_format_dds::DXGI_FORMAT_R32G32_UINT:					format = gl::format::r32g32_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R32G32_SINT:					format = gl::format::r32g32_sint; break;
		case dxgi_format_dds::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:			format = gl::format::d32_sfloat_s8_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R8G8B8A8_UNORM:				format = gl::format::r8g8b8a8_unorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:			format = gl::format::r8g8b8a8_srgb; break;
		case dxgi_format_dds::DXGI_FORMAT_R8G8B8A8_UINT:				format = gl::format::r8g8b8a8_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R8G8B8A8_SNORM:				format = gl::format::r8g8b8a8_snorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R8G8B8A8_SINT:				format = gl::format::r8g8b8a8_sint; break;
		case dxgi_format_dds::DXGI_FORMAT_R16G16_FLOAT:					format = gl::format::r16g16_sfloat; break;
		case dxgi_format_dds::DXGI_FORMAT_R16G16_UNORM:					format = gl::format::r16g16_unorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R16G16_UINT:					format = gl::format::r16g16_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R16G16_SNORM:					format = gl::format::r16g16_snorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R16G16_SINT:					format = gl::format::r16g16_sint; break;
		case dxgi_format_dds::DXGI_FORMAT_D32_FLOAT:					format = gl::format::d32_sfloat; break;
		case dxgi_format_dds::DXGI_FORMAT_R32_FLOAT:					format = gl::format::r32_sfloat; break;
		case dxgi_format_dds::DXGI_FORMAT_R32_UINT:						format = gl::format::r32_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R32_SINT:						format = gl::format::r32_sint; break;
		case dxgi_format_dds::DXGI_FORMAT_D24_UNORM_S8_UINT:			format = gl::format::d24_unorm_s8_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R8G8_UNORM:					format = gl::format::r8g8_unorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R8G8_UINT:					format = gl::format::r8g8_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R8G8_SNORM:					format = gl::format::r8g8_snorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R8G8_SINT:					format = gl::format::r8g8_sint; break;
		case dxgi_format_dds::DXGI_FORMAT_R16_FLOAT:					format = gl::format::r16_sfloat; break;
		case dxgi_format_dds::DXGI_FORMAT_D16_UNORM:					format = gl::format::d16_unorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R16_UNORM:					format = gl::format::r16_unorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R16_UINT:						format = gl::format::r16_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R16_SNORM:					format = gl::format::r16_snorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R16_SINT:						format = gl::format::r16_sint; break;
		case dxgi_format_dds::DXGI_FORMAT_R8_UNORM:						format = gl::format::r8_unorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R8_UINT:						format = gl::format::r8_uint; break;
		case dxgi_format_dds::DXGI_FORMAT_R8_SNORM:						format = gl::format::r8_snorm; break;
		case dxgi_format_dds::DXGI_FORMAT_R8_SINT:						format = gl::format::r8_sint; break;
		case dxgi_format_dds::DXGI_FORMAT_B5G6R5_UNORM:					format = gl::format::b5g6r5_unorm_pack16; break;
		case dxgi_format_dds::DXGI_FORMAT_B5G5R5A1_UNORM:				format = gl::format::b5g5r5a1_unorm_pack16; break;
		case dxgi_format_dds::DXGI_FORMAT_B8G8R8A8_UNORM:				format = gl::format::b8g8r8a8_unorm; break;
		case dxgi_format_dds::DXGI_FORMAT_B8G8R8X8_UNORM:				format = gl::format::b8g8r8a8_unorm; break;
		case dxgi_format_dds::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:			format = gl::format::b8g8r8a8_srgb; break;
		case dxgi_format_dds::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:			format = gl::format::b8g8r8a8_srgb; break;

		case dxgi_format_dds::DXGI_FORMAT_BC1_UNORM:					format = gl::format::bc1_rgba_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC1_UNORM_SRGB:				format = gl::format::bc1_rgba_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC2_UNORM:					format = gl::format::bc2_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC2_UNORM_SRGB:				format = gl::format::bc2_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC3_UNORM:					format = gl::format::bc3_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC3_UNORM_SRGB:				format = gl::format::bc3_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC4_UNORM:					format = gl::format::bc4_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC4_SNORM:					format = gl::format::bc4_snorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC5_UNORM:					format = gl::format::bc5_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC5_SNORM:					format = gl::format::bc5_snorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC6H_UF16:					format = gl::format::bc6h_ufloat_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC6H_SF16:					format = gl::format::bc6h_sfloat_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC7_UNORM:					format = gl::format::bc7_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_BC7_UNORM_SRGB:				format = gl::format::bc7_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_B4G4R4A4_UNORM:				format = gl::format::b4g4r4a4_unorm_pack16; break;

		case dxgi_format_dds::DXGI_FORMAT_ASTC_4X4_UNORM:				format = gl::format::astc_4x4_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_4X4_UNORM_SRGB:			format = gl::format::astc_4x4_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_5X4_UNORM:				format = gl::format::astc_5x4_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_5X4_UNORM_SRGB:			format = gl::format::astc_5x4_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_5X5_UNORM:				format = gl::format::astc_5x5_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_5X5_UNORM_SRGB:			format = gl::format::astc_5x5_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_6X5_UNORM:				format = gl::format::astc_6x5_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_6X5_UNORM_SRGB:			format = gl::format::astc_6x5_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_6X6_UNORM:				format = gl::format::astc_6x6_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_6X6_UNORM_SRGB:			format = gl::format::astc_6x6_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_8X5_UNORM:				format = gl::format::astc_8x5_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_8X5_UNORM_SRGB:			format = gl::format::astc_8x5_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_8X6_UNORM:				format = gl::format::astc_8x6_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_8X6_UNORM_SRGB:			format = gl::format::astc_8x6_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_8X8_UNORM:				format = gl::format::astc_8x8_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_8X8_UNORM_SRGB:			format = gl::format::astc_8x8_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_10X5_UNORM:				format = gl::format::astc_10x5_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_10X5_UNORM_SRGB:			format = gl::format::astc_10x5_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_10X6_UNORM:				format = gl::format::astc_10x6_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_10X6_UNORM_SRGB:			format = gl::format::astc_10x6_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_10X8_UNORM:				format = gl::format::astc_10x8_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_10X8_UNORM_SRGB:			format = gl::format::astc_10x8_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_10X10_UNORM:				format = gl::format::astc_10x10_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_10X10_UNORM_SRGB:		format = gl::format::astc_10x10_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_12X10_UNORM:				format = gl::format::astc_12x10_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_12X10_UNORM_SRGB:		format = gl::format::astc_12x10_srgb_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_12X12_UNORM:				format = gl::format::astc_12x12_unorm_block; break;
		case dxgi_format_dds::DXGI_FORMAT_ASTC_12X12_UNORM_SRGB:		format = gl::format::astc_12x12_srgb_block; break;
		}
	}

	if (!format) {
		using namespace attributes;
		ste_log_error() << text::attributed_string("DDS ") + i(lib::to_string(path.string())) + ": Unsupported format" << std::endl;
		throw surface_error("Unsupported DDS format");
	}

	// Create surface
	const auto mipmap_count = (header.Flags & DDSD_MIPMAPCOUNT) ? levels_t(header.MipMapLevels) : 1_mips;
	const std::uint32_t face_count = header.CubemapFlags & DDSCAPS2_CUBEMAP ? static_cast<std::uint32_t>(glm::bitCount(header.CubemapFlags & DDSCAPS2_CUBEMAP_ALLFACES)) : 1;
	const std::uint32_t depth_count = header.CubemapFlags & DDSCAPS2_VOLUME ? static_cast<std::uint32_t>(header.Depth) : 1;
	const auto array_size = std::max(layers_t(Header10.ArraySize), 1_layers);
	const auto layers = image_type == gl::image_type::image_cubemap || image_type == gl::image_type::image_cubemap_array ?
		array_size * face_count :
		array_size;

	if ((image_type == gl::image_type::image_cubemap || image_type == gl::image_type::image_cubemap_array) && 
		face_count != 6) {
		using namespace attributes;
		ste_log_error() << text::attributed_string("DDS ") + i(lib::to_string(path.string())) + ": Partial cubemaps are unsupported" << std::endl;
		throw surface_error("DDS: Partial cubemaps are unsupported");
	}

	extent_type extent;
	extent.x = header.Width;
	if constexpr (dimensions > 1)	extent.y = static_cast<std::uint32_t>(header.Height);
	if constexpr (dimensions > 2)	extent.z = depth_count;

	auto bytes = byte_t(content.size() - header_bytes);
	surface_t surface(format.get(),
					  image_type,
					  extent,
					  mipmap_count,
					  layers,
					  reinterpret_cast<const std::uint8_t*>(data),
					  bytes);
	assert(bytes == surface.bytes());

	return surface;
}

}

opaque_surface<1> surface_io::load_dds_1d(const std::experimental::filesystem::path &file_name) {
	return _detail::load_dds<1>(file_name);
}

opaque_surface<2> surface_io::load_dds_2d(const std::experimental::filesystem::path &file_name) {
	return _detail::load_dds<2>(file_name);
}

opaque_surface<3> surface_io::load_dds_3d(const std::experimental::filesystem::path &file_name) {
	return _detail::load_dds<3>(file_name);
}

void surface_io::write_dds(const std::experimental::filesystem::path &file_name, 
						   const std::uint8_t *image_data, 
						   byte_t bytes,
						   gl::format format, 
						   gl::image_type image_type, 
						   const glm::u32vec3 &extent, 
						   levels_t levels, 
						   layers_t layers) {
	const auto dimensions = gl::image_dimensions_for_type(image_type);
	const auto format_traits = gl::format_id(format);

	// Deduce target format
	optional<dxgi_format_dds> dxgi_format;
	dds_pixel_format dds_format;
	dds_format.magic = d3dfmt::D3DFMT_UNKNOWN;

	switch (format) {
	case gl::format::r4g4_unorm_pack8:			dds_format.Mask = glm::u32vec4{ 0x000F, 0x00F0, 0x0000, 0x0000 }; dds_format.magic = d3dfmt::D3DFMT_UNKNOWN; break;
	case gl::format::r4g4b4a4_unorm_pack16:		dds_format.Mask = glm::u32vec4{ 0x000F, 0x00F0, 0x0F00, 0xF000 }; dds_format.magic = d3dfmt::D3DFMT_UNKNOWN; break;
	case gl::format::b4g4r4a4_unorm_pack16:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_B4G4R4A4_UNORM; break;
	case gl::format::r5g6b5_unorm_pack16:		dds_format.Mask = glm::u32vec4{ 0x001f, 0x07e0, 0xf800, 0x0000 }; dds_format.magic = d3dfmt::D3DFMT_UNKNOWN; break;
	case gl::format::b5g6r5_unorm_pack16:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_B5G6R5_UNORM; break;
	case gl::format::r5g5b5a1_unorm_pack16:		dds_format.Mask = glm::u32vec4{ 0x001f, 0x03e0, 0x7c00, 0x8000 }; dds_format.magic = d3dfmt::D3DFMT_UNKNOWN; break;
	case gl::format::b5g5r5a1_unorm_pack16:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_B5G5R5A1_UNORM; break;
	case gl::format::r8_unorm:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8_UNORM; break;
	case gl::format::r8_snorm:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8_SNORM; break;
	case gl::format::r8_uint:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8_UINT; break;
	case gl::format::r8_sint:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8_SINT; break;
	case gl::format::r8g8_unorm:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8G8_UNORM; break;
	case gl::format::r8g8_snorm:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8G8_SNORM; break;
	case gl::format::r8g8_uint:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8G8_UINT; break;
	case gl::format::r8g8_sint:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8G8_SINT; break;
	case gl::format::r8g8b8_unorm:				dds_format.Mask = glm::u32vec4{ 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 }; dds_format.magic = d3dfmt::D3DFMT_UNKNOWN; break;
	case gl::format::b8g8r8_unorm:				dds_format.Mask = glm::u32vec4{ 0, 0, 0, 0 }; dds_format.magic = d3dfmt::D3DFMT_R8G8B8; break;
	case gl::format::r8g8b8a8_unorm:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8G8B8A8_UNORM; break;
	case gl::format::r8g8b8a8_snorm:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8G8B8A8_SNORM; break;
	case gl::format::r8g8b8a8_uint:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8G8B8A8_UINT; break;
	case gl::format::r8g8b8a8_sint:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8G8B8A8_SINT; break;
	case gl::format::r8g8b8a8_srgb:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
	case gl::format::b8g8r8a8_unorm:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_B8G8R8A8_UNORM; break;
	case gl::format::b8g8r8a8_srgb:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; break;
	case gl::format::r16_unorm:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16_UNORM; break;
	case gl::format::r16_snorm:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16_SNORM; break;
	case gl::format::r16_uint:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16_UINT; break;
	case gl::format::r16_sint:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16_SINT; break;
	case gl::format::r16_sfloat:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16_FLOAT; break;
	case gl::format::r16g16_unorm:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16G16_UNORM; break;
	case gl::format::r16g16_snorm:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16G16_SNORM; break;
	case gl::format::r16g16_uint:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16G16_UINT; break;
	case gl::format::r16g16_sint:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16G16_SINT; break;
	case gl::format::r16g16_sfloat:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16G16_FLOAT; break;
	case gl::format::r16g16b16a16_unorm:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16G16B16A16_UNORM; break;
	case gl::format::r16g16b16a16_snorm:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16G16B16A16_SNORM; break;
	case gl::format::r16g16b16a16_uint:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16G16B16A16_UINT; break;
	case gl::format::r16g16b16a16_sint:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16G16B16A16_SINT; break;
	case gl::format::r16g16b16a16_sfloat:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_R16G16B16A16_FLOAT; break;
	case gl::format::r32_uint:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32_UINT; break;
	case gl::format::r32_sint:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32_SINT; break;
	case gl::format::r32_sfloat:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32_FLOAT; break;
	case gl::format::r32g32_uint:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32G32_UINT; break;
	case gl::format::r32g32_sint:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32G32_SINT; break;
	case gl::format::r32g32_sfloat:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32G32_FLOAT; break;
	case gl::format::r32g32b32_uint:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32G32B32_UINT; break;
	case gl::format::r32g32b32_sint:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32G32B32_SINT; break;
	case gl::format::r32g32b32_sfloat:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32G32B32_FLOAT; break;
	case gl::format::r32g32b32a32_uint:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32G32B32A32_UINT; break;
	case gl::format::r32g32b32a32_sint:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32G32B32A32_SINT; break;
	case gl::format::r32g32b32a32_sfloat:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_R32G32B32A32_FLOAT; break;
	case gl::format::d16_unorm:					dxgi_format = dxgi_format_dds::DXGI_FORMAT_D16_UNORM; break;
	case gl::format::d32_sfloat:				dxgi_format = dxgi_format_dds::DXGI_FORMAT_D32_FLOAT; break;
	case gl::format::d24_unorm_s8_uint:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_D24_UNORM_S8_UINT; break;
	case gl::format::d32_sfloat_s8_uint:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_D32_FLOAT_S8X24_UINT; break;
	case gl::format::bc1_rgba_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC1_UNORM; break;
	case gl::format::bc1_rgba_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC1_UNORM_SRGB; break;
	case gl::format::bc2_unorm_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC2_UNORM; break;
	case gl::format::bc2_srgb_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC2_UNORM_SRGB; break;
	case gl::format::bc3_unorm_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC3_UNORM; break;
	case gl::format::bc3_srgb_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC3_UNORM_SRGB; break;
	case gl::format::bc4_unorm_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC4_UNORM; break;
	case gl::format::bc4_snorm_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC4_SNORM; break;
	case gl::format::bc5_unorm_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC5_UNORM; break;
	case gl::format::bc5_snorm_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC5_SNORM; break;
	case gl::format::bc6h_ufloat_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC6H_UF16; break;
	case gl::format::bc6h_sfloat_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC6H_SF16; break;
	case gl::format::bc7_unorm_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC7_UNORM; break;
	case gl::format::bc7_srgb_block:			dxgi_format = dxgi_format_dds::DXGI_FORMAT_BC7_UNORM_SRGB; break;
	case gl::format::astc_4x4_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_4X4_UNORM; break;
	case gl::format::astc_4x4_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_4X4_UNORM_SRGB; break;
	case gl::format::astc_5x4_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_5X4_UNORM; break;
	case gl::format::astc_5x4_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_5X4_UNORM_SRGB; break;
	case gl::format::astc_5x5_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_5X5_UNORM; break;
	case gl::format::astc_5x5_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_5X5_UNORM_SRGB; break;
	case gl::format::astc_6x5_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_6X5_UNORM; break;
	case gl::format::astc_6x5_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_6X5_UNORM_SRGB; break;
	case gl::format::astc_6x6_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_6X6_UNORM; break;
	case gl::format::astc_6x6_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_6X6_UNORM_SRGB; break;
	case gl::format::astc_8x5_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_8X5_UNORM; break;
	case gl::format::astc_8x5_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_8X5_UNORM_SRGB; break;
	case gl::format::astc_8x6_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_8X6_UNORM; break;
	case gl::format::astc_8x6_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_8X6_UNORM_SRGB; break;
	case gl::format::astc_8x8_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_8X8_UNORM; break;
	case gl::format::astc_8x8_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_8X8_UNORM_SRGB; break;
	case gl::format::astc_10x5_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_10X5_UNORM; break;
	case gl::format::astc_10x5_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_10X5_UNORM_SRGB; break;
	case gl::format::astc_10x6_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_10X6_UNORM; break;
	case gl::format::astc_10x6_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_10X6_UNORM_SRGB; break;
	case gl::format::astc_10x8_unorm_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_10X8_UNORM; break;
	case gl::format::astc_10x8_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_10X8_UNORM_SRGB; break;
	case gl::format::astc_10x10_unorm_block:	dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_10X10_UNORM; break;
	case gl::format::astc_10x10_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_10X10_UNORM_SRGB; break;
	case gl::format::astc_12x10_unorm_block:	dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_12X10_UNORM; break;
	case gl::format::astc_12x10_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_12X10_UNORM_SRGB; break;
	case gl::format::astc_12x12_unorm_block:	dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_12X12_UNORM; break;
	case gl::format::astc_12x12_srgb_block:		dxgi_format = dxgi_format_dds::DXGI_FORMAT_ASTC_12X12_UNORM_SRGB; break;
	default: {
		using namespace attributes;
		ste_log_error() << "Surface format incompatible with DDS.";
		throw surface_unsupported_format_error("Surface format incompatible with DDS");
	}
	}

	dds_format.bpp = static_cast<std::uint32_t>(format_traits.block_bytes) * 8;
	if (format_traits.elements < 4)		dds_format.flags = DDPF_RGB;
	else								dds_format.flags = DDPF_RGBA;
	if (dxgi_format) {
		dds_format.magic = d3dfmt::D3DFMT_DX10;
		dds_format.flags = DDPF_FOURCC;
	}

	// Can not save arrays or 1D surface with old format
	if (!dxgi_format && (layers > 1_layers || gl::image_dimensions_for_type(image_type) == 1)) {
		using namespace attributes;
		ste_log_error() << "Surface type and format incompatible with DDS.";
		throw surface_unsupported_format_error("Surface type and format incompatible with DDS");
	}

	// Create header buffer
	static constexpr int header_buffer_size = sizeof(magic_dds) + sizeof(dds_header) + sizeof(dds_header10);
	std::uint8_t header[header_buffer_size];
	std::memcpy(header, magic_dds, sizeof(magic_dds));
	dds_header& Header = *reinterpret_cast<dds_header*>(header + sizeof(magic_dds));
	dds_header10& Header10 = *reinterpret_cast<dds_header10*>(header + sizeof(magic_dds) + sizeof(dds_header));

	// Populate headers
	std::uint32_t Caps = DDSD_CAPS | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
	Caps |= dimensions > 1 ? DDSD_HEIGHT : 0;
	Caps |= dimensions > 2 ? DDSD_DEPTH : 0;
	Caps |= format_traits.is_compressed ? DDSD_LINEARSIZE : DDSD_PITCH;

	std::memset(Header.Reserved1, 0, sizeof(Header.Reserved1));
	std::memset(Header.Reserved2, 0, sizeof(Header.Reserved2));
	Header.Size = sizeof(dds_header);
	Header.Flags = Caps;
	Header.Width = extent.x;
	Header.Height = extent.y;
	Header.Depth = extent.z > 1 ? extent.z : 0;
	Header.Pitch = static_cast<std::uint32_t>(format_traits.is_compressed ? std::max(1_B, format_traits.block_bytes * (extent.x + 3) / 4) : byte_t(extent.x * dds_format.bpp + 7) / 8);
	Header.MipMapLevels = static_cast<std::uint32_t>(levels);
	Header.Format = dds_format;
	Header.SurfaceFlags = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
	Header.CubemapFlags = 0;
	if (image_type == gl::image_type::image_cubemap || image_type == gl::image_type::image_cubemap_array) {
		Header.CubemapFlags |= DDSCAPS2_CUBEMAP_ALLFACES | DDSCAPS2_CUBEMAP;
	}
	if (extent.z > 1)
		Header.CubemapFlags |= DDSCAPS2_VOLUME;

	if (dxgi_format) {
		Header10.ArraySize = static_cast<std::uint32_t>(layers);
		Header10.ResourceDimension = dimensions == 1 ? D3D10_RESOURCE_DIMENSION_TEXTURE1D : (dimensions == 2 ? D3D10_RESOURCE_DIMENSION_TEXTURE2D : D3D10_RESOURCE_DIMENSION_TEXTURE3D);
		Header10.MiscFlag = 0;
		Header10.Format = dxgi_format.get();
		Header10.AlphaFlags = DDS_ALPHA_MODE_UNKNOWN;
	}

	// Write out
	{
		std::ofstream fs(file_name.string(), std::ios::out | std::ios::binary);
		if (!fs) {
			using namespace attributes;
			ste_log_error() << text::attributed_string("Can't open file ") + i(lib::to_string(file_name.string())) + " for writing: " + std::strerror(errno) << std::endl;
			throw resource_io_error("Could not open file");
		}

		// Write headers
		std::copy(header, header + header_buffer_size, std::ostream_iterator<std::uint8_t>(fs));
		// Write image data
		std::copy(image_data, image_data + static_cast<std::size_t>(bytes), std::ostream_iterator<std::uint8_t>(fs));
	}
}
