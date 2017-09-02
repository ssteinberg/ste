
#include <stdafx.hpp>
#include <surface_io.hpp>

#include <log.hpp>
#include <attributed_string.hpp>
#include <attrib.hpp>

#include <lib/unique_ptr.hpp>

#include <libpng16/png.h>

using namespace ste;
using namespace ste::text;
using namespace ste::resource;

opaque_surface<2> surface_io::load_png_2d(const std::experimental::filesystem::path &file_name, bool srgb) {
	png_byte header[8];

	FILE *fp = fopen(file_name.string().data(), "rb");
	if (!fp) {
		throw resource_io_error("Could not open file");
	}

	// read the header
	fread(header, 1, 8, fp);

	if (png_sig_cmp(header, 0, 8)) {
		ste_log_error() << file_name << " is not a PNG" << std::endl;
		fclose(fp);
		throw surface_unsupported_format_error("Not a valid PNG");
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) {
		ste_log_error() << file_name << " png_create_read_struct returned 0" << std::endl;
		fclose(fp);
		throw surface_unsupported_format_error("Not a valid PNG");
	}

	// create png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		ste_log_error() << file_name << " png_create_info_struct returned 0" << std::endl;
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		fclose(fp);
		throw surface_unsupported_format_error("Not a valid PNG");
	}

	// create png info struct
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		ste_log_error() << file_name << " png_create_info_struct returned 0" << std::endl;
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		fclose(fp);
		throw surface_unsupported_format_error("Not a valid PNG");
	}

	// the code in this if statement gets called if libpng encounters an error
	if (setjmp(png_jmpbuf(png_ptr))) {
		ste_log_error() << file_name << " error from libpng" << std::endl;
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		throw surface_error("libpng error");
	}

	// init png reading
	png_init_io(png_ptr, fp);

	// let libpng know you already read the first 8 bytes
	png_set_sig_bytes(png_ptr, 8);

	// read all the info up to the image data
	png_read_info(png_ptr, info_ptr);

	// variables to pass to get info
	int bit_depth, color_type;
	png_uint_32 temp_width, temp_height;

	// get info about png
	png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
				 nullptr, nullptr, nullptr);

	if (bit_depth != 8 && (bit_depth != 1 || color_type != PNG_COLOR_TYPE_GRAY)) {
		ste_log_error() << file_name << " Unsupported bit depth " << bit_depth << ".  Must be 8" << std::endl;
		throw surface_unsupported_format_error("Unsupported bit depth");
	}

	gl::format format;
	int components;
	switch (color_type) {
	case PNG_COLOR_TYPE_GRAY:
		format = srgb ? gl::format::r8_srgb : gl::format::r8_unorm;
		components = 1;
		break;
	case PNG_COLOR_TYPE_RGB:
		format = srgb ? gl::format::r8g8b8_srgb : gl::format::r8g8b8_unorm;
		components = 3;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		format = srgb ? gl::format::r8g8b8a8_srgb : gl::format::r8g8b8a8_unorm;
		components = 4;
		break;
	default:
		ste_log_error() << file_name << " Unknown libpng color type " << color_type << std::endl;
		fclose(fp);
		throw surface_unsupported_format_error("Unsupported PNG color type");
	}

	// Update the png info struct.
	png_read_update_info(png_ptr, info_ptr);

	// Row size in bytes.
	auto rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	if (bit_depth == 1) rowbytes *= 8;

	// glTexImage2d requires rows to be 4-byte aligned
	if (3 - ((rowbytes - 1) % 4))
		ste_log_warn() << file_name << " image row not 4byte aligned!";
	rowbytes += 3 - ((rowbytes - 1) % 4);

	// Allocate the image_data as a big block
	const std::size_t w = rowbytes / components + !!(rowbytes%components);
	const auto level0_size = rowbytes * temp_height;
	lib::unique_ptr<std::uint8_t[]> image_data = lib::allocate_unique<std::uint8_t[]>(level0_size);

	// row_pointers is for pointing to image_data for reading the png with libpng
	png_byte ** row_pointers = reinterpret_cast<png_byte **>(malloc(temp_height * sizeof(png_byte *)));
	if (bit_depth == 8) {
		// set the individual row_pointers to point at the correct offsets of image_data
		for (unsigned int i = 0; i < temp_height; i++)
			row_pointers[temp_height - 1 - i] = reinterpret_cast<png_byte*>(image_data.get() + i * w * components);

		// read the png into image_data through row_pointers
		png_read_image(png_ptr, row_pointers);
	}
	else if (bit_depth == 1) {
		png_byte *temp = new png_byte[w * temp_height / 8 + 1];

		// set the individual row_pointers to point at the correct offsets of image_data
		for (unsigned int i = 0; i < temp_height; i++)
			row_pointers[temp_height - 1 - i] = reinterpret_cast<png_byte*>(temp + i * w / 8);

		// read the png into image_data through row_pointers
		png_read_image(png_ptr, row_pointers);

		for (unsigned i = 0; i < w * temp_height; ++i) {
			const int j = i / 8;
			const int bit = i % 8;
			const char byte = static_cast<char>(temp[j]);
			image_data[i] = (!!(byte & (0x1 << (8 - bit)))) * 255;
		}

		delete[] temp;
	}

	fclose(fp);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	free(row_pointers);

	opaque_surface<2> tex(format,
						  gl::image_type::image_2d,
						  { w, temp_height },
						  1, 1,
						  std::move(image_data),
						  level0_size);
	return tex;
}

void surface_io::write_png_2d(const std::experimental::filesystem::path &file_name, const std::uint8_t *image_data, int components, int width, int height) {
	FILE *fp = fopen(file_name.string().data(), "wb");
	if (!fp) {
		ste_log_error() << file_name << " can't be opened for writing" << std::endl;
		throw resource_io_error("Opening output file failed");
	}

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png) {
		ste_log_error() << file_name << " png_create_write_struct failed" << std::endl;
		fclose(fp);
		throw surface_error("png_create_write_struct failed");
	}

	png_infop info = png_create_info_struct(png);
	if (!info) {
		ste_log_error() << file_name << " png_create_info_struct failed" << std::endl;
		fclose(fp);
		throw surface_error("png_create_info_struct failed");
	}

	if (setjmp(png_jmpbuf(png))) {
		ste_log_error() << file_name << " png_jmpbuf failed" << std::endl;
		fclose(fp);
		throw surface_error("png_jmpbuf failed");
	}

	png_byte ** const row_pointers = reinterpret_cast<png_byte **>(malloc(height * sizeof(png_byte *)));
	if (row_pointers == nullptr) {
		ste_log_error() << file_name << " could not allocate memory for PNG row pointers" << std::endl;
		fclose(fp);
		throw surface_error("Could not allocate memory for PNG row pointers");
	}

	// set the individual row_pointers to point at the correct offsets of image_data
	// To maintain compatibility png_write_image requests a non-const double pointer, hack the const away...
	for (int i = 0; i < height; i++)
		row_pointers[height - 1 - i] = const_cast<png_byte*>(reinterpret_cast<const png_byte*>(image_data + i * width * components));

	png_init_io(png, fp);

	int color_type;
	switch (components) {
	case 1: color_type = PNG_COLOR_TYPE_GRAY; break;
	case 3: color_type = PNG_COLOR_TYPE_RGB; break;
	case 4: color_type = PNG_COLOR_TYPE_RGBA; break;
	default: {
		using namespace attributes;
		ste_log_error() << i(lib::to_string(file_name.string())) << " can't write " << components << " channel PNG.";
		throw surface_unsupported_format_error("Unsupported PNG component count");
	}
	}
	png_set_IHDR(png,
				 info,
				 width, height,
				 8,
				 color_type,
				 PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT,
				 PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

	png_write_image(png, row_pointers);
	png_write_end(png, nullptr);

	free(row_pointers);

	fclose(fp);
	
}
