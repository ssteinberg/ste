
#include <stdafx.hpp>
#include <surface_io.hpp>

#include <log.hpp>
#include <attributed_string.hpp>
#include <attrib.hpp>

#include <lib/unique_ptr.hpp>
#include <lib/vector.hpp>

#include <tga.h>

using namespace ste;
using namespace ste::text;
using namespace ste::resource;

opaque_surface<2> surface_io::load_tga_2d(const std::experimental::filesystem::path &file_name, bool srgb) {
	// Open for reading and read header
	TGA *tga = TGAOpen(const_cast<char*>(file_name.string().data()), const_cast<char*>("rb"));
	if (!tga || tga->last != TGA_OK ||
		TGAReadHeader(tga) != TGA_OK) {
		if (tga)
			TGAClose(tga);
		ste_log_error() << file_name << " is not a valid 24-bit TGA" << std::endl;
		throw surface_unsupported_format_error("Not a valid 24-bit TGA");
	}

	// Choose format
	const unsigned w = tga->hdr.width;
	const unsigned h = tga->hdr.height;
	gl::format format;
	int components;
	switch (tga->hdr.img_t) {
	case 2:
	case 3:
	case 10:
	case 11:
		if (tga->hdr.depth == 8 && 
			(tga->hdr.alpha == 0 || tga->hdr.alpha == 8)) {
			if (tga->hdr.alpha == 8) {
				// 8-bit depth and 8-bit alpha textures are masks, height maps, etc. sRGB makes no sense.
				assert(!srgb);
			}

			format = srgb ? gl::format::r8_srgb : gl::format::r8_unorm;
			components = 1;
			break;
		}
		if (tga->hdr.depth == 24 && 
			tga->hdr.alpha == 0) {
			format = srgb ? gl::format::r8g8b8_srgb : gl::format::r8g8b8_unorm;
			components = 3;
			break;
		}
		if (tga->hdr.depth == 32 && 
			tga->hdr.alpha == 8) {
			format = srgb ? gl::format::r8g8b8a8_srgb : gl::format::r8g8b8a8_unorm;
			components = 4;
			break;
		}
	default:
		TGAClose(tga);
		ste_log_error() << file_name << " Unsupported libtga depth (" << tga->hdr.depth << ") and image type (" << tga->hdr.img_t << ") combination" << std::endl;
		throw surface_unsupported_format_error("Unsupported TGA depth/type combination");
	}

	// Read
	unsigned rowbytes = w * components;
	if (3 - ((rowbytes - 1) % 4))
		ste_log_warn() << file_name << " image row not 4byte aligned!";
	rowbytes += 3 - ((rowbytes - 1) % 4);

	const auto level0_size = rowbytes * h;
	lib::unique_ptr<std::uint8_t[]> image_data = lib::allocate_unique<std::uint8_t[]>(level0_size);

	if (TGAReadScanlines(tga, 
						 reinterpret_cast<tbyte*>(image_data.get()), 
						 0,
						 h, 
						 TGA_RGB) != h) {
		ste_log_error() << file_name << " is not a valid TGA" << std::endl;
		throw surface_unsupported_format_error("Not a valid TGA");
	}

	// Flip vertical
	if (tga->hdr.vert != TGA_TOP) {
		lib::vector<std::uint8_t> buffer;
		buffer.resize(rowbytes);
		for (unsigned y = 0; y < h/2; ++y) {
			const auto lower_row = image_data.get() + y * rowbytes;
			const auto upper_row = image_data.get() + (h - y - 1) * rowbytes;

			std::memcpy(buffer.data(), lower_row, rowbytes);
			std::memcpy(lower_row, upper_row, rowbytes);
			std::memcpy(upper_row, buffer.data(), rowbytes);
		}
	}

	TGAClose(tga);

	// Create surface
	opaque_surface<2> tex(format, 
						  gl::image_type::image_2d, 
						  { w, h }, 
						  1, 1, 
						  std::move(image_data), 
						  level0_size);
	return tex;
}

void surface_io::write_tga_2d(const std::experimental::filesystem::path &file_name, const std::uint8_t *image_data, int components, int width, int height) {
	// Open for writing
	TGA *tga = TGAOpen(const_cast<char*>(file_name.string().data()), const_cast<char*>("wb"));
	if (!tga || tga->last != TGA_OK) {
		if (tga)
			TGAClose(tga);

		using namespace attributes;
		ste_log_error() << "Failed opening " << i(lib::to_string(file_name.string())) << " for writing" << std::endl;
		throw resource_io_error("TGAOpen failed");
	}

	// Construct TGA data
	TGAData data;
	data.flags = TGA_RGB | TGA_IMAGE_DATA;
	data.img_data = const_cast<tbyte*>(reinterpret_cast<const tbyte*>(image_data));
	data.img_id = nullptr;
	data.cmap = nullptr;

	std::memset(&tga->hdr, 0, sizeof(tga->hdr));
	tga->hdr.width = width;
	tga->hdr.height = height;
	tga->hdr.horz = TGA_LEFT;
	tga->hdr.vert = TGA_TOP;
	tga->hdr.img_t = 2;
	switch (components) {
	case 1:
		tga->hdr.alpha = 0;
		tga->hdr.depth = 8;
		break;
	case 3:
		tga->hdr.alpha = 0;
		tga->hdr.depth = 24;
		break;
	case 4:
		tga->hdr.alpha = 8;
		tga->hdr.depth = 32;
		break;
	default:
		ste_log_error() << file_name << " can't write " << components << " channel TGA.";
		throw surface_unsupported_format_error("Unsupported TGA component count");
	}

	// Write
	if (TGAWriteImage(tga, &data) != TGA_OK) {
		using namespace attributes;
		ste_log_error() << "Failed writing TGA image to " << i(lib::to_string(file_name.string())) << std::endl;
		throw resource_io_error("Failed writing TGA image");
	}

	TGAClose(tga);
}
