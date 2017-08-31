
#include <stdafx.hpp>
#include <surface_io.hpp>

#include <log.hpp>
#include <attributed_string.hpp>
#include <attrib.hpp>

#include <lib/vector.hpp>

#include <tga.h>

using namespace ste;
using namespace ste::text;
using namespace ste::resource;

opaque_surface<2> surface_io::load_tga(const std::experimental::filesystem::path &file_name, bool srgb) {
	TGA *tga;

	try {
		tga = TGAOpen(const_cast<char*>(file_name.string().data()), const_cast<char*>("rb"));
		TGAReadHeader(tga);
	}
	catch (const std::exception &) {
		ste_log_error() << file_name << " is not a valid 24-bit TGA" << std::endl;
		throw resource_io_error("TGAOpen failed");
	}

	if (tga->last != TGA_OK) {
		TGAClose(tga);
		ste_log_error() << file_name << " is not a valid 24-bit TGA" << std::endl;
		throw surface_unsupported_format_error("Not a valid 24-bit TGA");
	}

	const unsigned w = tga->hdr.width;
	const unsigned h = tga->hdr.height;
	gl::format format;
	int components;
	switch (tga->hdr.img_t) {
	case 2:
	case 3:
	case 10:
	case 11:
		if (tga->hdr.depth == 8) {
			format = srgb ? gl::format::r8_srgb : gl::format::r8_unorm;
			components = 1;
			break;
		}
		if (tga->hdr.depth == 24) {
			format = srgb ? gl::format::r8g8b8_srgb : gl::format::r8g8b8_unorm;
			components = 3;
			break;
		}
		if (tga->hdr.depth == 32) {
			format = srgb ? gl::format::r8g8b8a8_srgb : gl::format::r8g8b8a8_unorm;
			components = 4;
			break;
		}
	default:
		TGAClose(tga);
		ste_log_error() << file_name << " Unsupported libtga depth (" << tga->hdr.depth << ") and image type (" << tga->hdr.img_t << ") combination" << std::endl;
		throw surface_unsupported_format_error("Unsupported TGA depth/type combination");
	}

	unsigned rowbytes = w * components;
	if (3 - ((rowbytes - 1) % 4))
		ste_log_warn() << file_name << " image row not 4byte aligned!";
	rowbytes += 3 - ((rowbytes - 1) % 4);

	const auto level0_size = rowbytes * h;
	lib::vector<std::uint8_t> image_data;
	image_data.resize(level0_size);

	try {
		TGAReadScanlines(tga, reinterpret_cast<tbyte*>(image_data.data()), 0, h, TGA_BGR);
	}
	catch (const std::exception &) {
		ste_log_error() << file_name << " is not a valid TGA" << std::endl;
		throw surface_unsupported_format_error("Not a valid TGA");
	}

	TGAClose(tga);

	opaque_surface<2> tex(format, gl::image_type::image_2d, { w, h }, 1, 1, std::move(image_data));
	return tex;
}
