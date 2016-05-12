
#include "stdafx.hpp"
#include "SurfaceFactory.hpp"

#include "Log.hpp"
#include "AttributedString.hpp"
#include "attrib.hpp"

#include <libpng16/png.h>
#include <turbojpeg.h>
#include <tga.h>

using namespace StE::Text;
using namespace StE::Resource;

bool SurfaceFactory::write_png(const boost::filesystem::path &file_name, const char *image_data, int components, int width, int height) {
	if (components != 1 && components != 3 && components != 4) {
		ste_log_error() << file_name << " can't write " << components << " channel PNG.";
		return false;
	}

	FILE *fp = fopen(file_name.string().data(), "wb");
	if (!fp) {
		ste_log_error() << file_name << " can't be opened for writing";
		return false;
	}

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		ste_log_error() << file_name << " png_create_write_struct failed";
		fclose(fp);
		return false;
	}

	png_infop info = png_create_info_struct(png);
	if (!info) {
		ste_log_error() << file_name << " png_create_info_struct failed";
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png))) {
		ste_log_error() << file_name << " png_jmpbuf failed";
		fclose(fp);
		return false;
	}

	png_byte ** const row_pointers = (png_byte **)malloc(height * sizeof(png_byte *));
	if (row_pointers == NULL) {
		ste_log_error() << file_name << " could not allocate memory for PNG row pointers";
		fclose(fp);
		return false;
	}

	// set the individual row_pointers to point at the correct offsets of image_data
	// To maintain compatability png_write_image requests a non-const double pointer, hack the const away...
	for (int i = 0; i < height; i++)
		row_pointers[height - 1 - i] = const_cast<png_byte*>(reinterpret_cast<const png_byte*>(image_data + i * width * components));

	png_init_io(png, fp);

	int color_type;
	switch (components) {
	case 1: color_type = PNG_COLOR_TYPE_GRAY; break;
	case 3: color_type = PNG_COLOR_TYPE_RGB; break;
	case 4: color_type = PNG_COLOR_TYPE_RGBA; break;
	default:
		break;
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
	png_write_end(png, NULL);

	free(row_pointers);

	fclose(fp);

	return true;
}

gli::texture2d SurfaceFactory::load_tga(const boost::filesystem::path &file_name, bool srgb) {
	TGA *tga;

	try {
		tga = TGAOpen(const_cast<char*>(file_name.string().data()), const_cast<char*>("rb"));
		TGAReadHeader(tga);
	}
	catch (const std::exception &ex) {
		ste_log_error() << file_name << " is not a valid 24-bit TGA" << std::endl;
		return gli::texture2d();
	}

	if (tga->last != TGA_OK) {
		TGAClose(tga);
		ste_log_error() << file_name << " is not a valid 24-bit TGA" << std::endl;
		return gli::texture2d();
	}

	unsigned w = tga->hdr.width;
	unsigned h = tga->hdr.height;
	gli::format format;
	int components;
	switch (tga->hdr.img_t) {
	case 2:
	case 3:
	case 10:
	case 11:
		if (tga->hdr.depth == 8) {
			format = srgb ? gli::format::FORMAT_R8_SRGB_PACK8 : gli::format::FORMAT_R8_UNORM_PACK8;
			components = 1;
			break;
		}
		else if (tga->hdr.depth == 24) {
			format = srgb ? gli::format::FORMAT_BGR8_SRGB_PACK8 : gli::format::FORMAT_BGR8_UNORM_PACK8;
			components = 3;
			break;
		}
		else if (tga->hdr.depth == 32) {
			format = srgb ? gli::format::FORMAT_BGRA8_SRGB_PACK8 : gli::format::FORMAT_BGRA8_UNORM_PACK8;
			components = 4;
			break;
		}
	default:
		TGAClose(tga);
		ste_log_error() << file_name << " Unsupported libtga depth (" << tga->hdr.depth << ") and image type (" << tga->hdr.img_t << ") combination" << std::endl;
		return gli::texture2d();
	}

	unsigned rowbytes = w * components;
	int rowsize = rowbytes;
	if (3 - ((rowbytes - 1) % 4))
		ste_log_warn() << file_name << " image row not 4byte aligned!";
	rowbytes += 3 - ((rowbytes - 1) % 4);

	w = rowbytes / components + !!(rowbytes%components);
	gli::texture2d tex(format, { w, h }, 1);
	tbyte *image_data = reinterpret_cast<tbyte*>(tex.data());
	auto level0_size = tex[0].size();
	if (image_data == nullptr || level0_size < rowbytes*h) {
		TGAClose(tga);
		ste_log_error() << file_name << " could not allocate memory for TGA image data or format mismatch";
		return gli::texture2d();
	}

	try {
		TGAReadScanlines(tga, image_data, 0, h, TGA_BGR);
	}
	catch (const std::exception &ex) {
		ste_log_error() << file_name << " is not a valid 24-bit TGA" << std::endl;
		return gli::texture2d();
	}

	TGAClose(tga);

	return tex;
}

gli::texture2d SurfaceFactory::load_png(const boost::filesystem::path &file_name, bool srgb) {
	png_byte header[8];

	FILE *fp = fopen(file_name.string().data(), "rb");
	if (fp == 0) {
		perror(file_name.string().data());
		return gli::texture2d();
	}

	// read the header
	fread(header, 1, 8, fp);

	if (png_sig_cmp(header, 0, 8)) {
		ste_log_error() << file_name << " is not a PNG";
		fclose(fp);
		return gli::texture2d();
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		ste_log_error() << file_name << " png_create_read_struct returned 0";
		fclose(fp);
		return gli::texture2d();
	}

	// create png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		ste_log_error() << file_name << " png_create_info_struct returned 0";
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(fp);
		return gli::texture2d();
	}

	// create png info struct
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		ste_log_error() << file_name << " png_create_info_struct returned 0";
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(fp);
		return gli::texture2d();
	}

	// the code in this if statement gets called if libpng encounters an error
	if (setjmp(png_jmpbuf(png_ptr))) {
		ste_log_error() << file_name << " error from libpng";
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return gli::texture2d();
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
		NULL, NULL, NULL);

	//printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);

	if (bit_depth != 8 && (bit_depth != 1 || color_type != PNG_COLOR_TYPE_GRAY)) {
		ste_log_error() << file_name << " Unsupported bit depth " << bit_depth << ".  Must be 8";
		return gli::texture2d();
	}

	gli::format format;
	int components;
	switch (color_type) {
	case PNG_COLOR_TYPE_GRAY:
		format = srgb ? gli::format::FORMAT_R8_SRGB_PACK8 : gli::format::FORMAT_R8_UNORM_PACK8;
		components = 1;
		break;
	case PNG_COLOR_TYPE_RGB:
		format = srgb ? gli::format::FORMAT_RGB8_SRGB_PACK8 : gli::format::FORMAT_RGB8_UNORM_PACK8;
		components = 3;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		format = srgb ? gli::format::FORMAT_RGBA8_SRGB_PACK8 : gli::format::FORMAT_RGBA8_UNORM_PACK8;
		components = 4;
		break;
	default:
		ste_log_error() << file_name << " Unknown libpng color type " << color_type;
		fclose(fp);
		return gli::texture2d();
	}

	// Update the png info struct.
	png_read_update_info(png_ptr, info_ptr);

	// Row size in bytes.
	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	if (bit_depth == 1) rowbytes *= 8;

	// glTexImage2d requires rows to be 4-byte aligned
	if (3 - ((rowbytes - 1) % 4))
		ste_log_warn() << file_name << " image row not 4byte aligned!";
	rowbytes += 3 - ((rowbytes - 1) % 4);

	// Allocate the image_data as a big block, to be given to opengl
	/*char * image_data = new char[rowbytes * temp_height * sizeof(png_byte) + 15];*/
	unsigned w = rowbytes / components + !!(rowbytes%components);
	gli::texture2d tex(format, { w, temp_height }, 1);
	char *image_data = reinterpret_cast<char*>(tex.data());
	auto level0_size = tex[0].size();
	if (image_data == nullptr || level0_size < rowbytes*temp_height) {
		ste_log_error() << file_name << " could not allocate memory for PNG image data or format mismatch";
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return gli::texture2d();
	}

	// row_pointers is for pointing to image_data for reading the png with libpng
	png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
	if (bit_depth == 8) {
		// set the individual row_pointers to point at the correct offsets of image_data
		for (unsigned int i = 0; i < temp_height; i++)
			row_pointers[temp_height - 1 - i] = reinterpret_cast<png_byte*>(image_data + i * w * components);

		// read the png into image_data through row_pointers
		png_read_image(png_ptr, row_pointers);
	}
	else if (bit_depth==1) {
		png_byte *temp = new png_byte[w * temp_height / 8 + 1];

		// set the individual row_pointers to point at the correct offsets of image_data
		for (unsigned int i = 0; i < temp_height; i++)
			row_pointers[temp_height - 1 - i] = reinterpret_cast<png_byte*>(temp + i * w / 8);

		// read the png into image_data through row_pointers
		png_read_image(png_ptr, row_pointers);

		for (unsigned i = 0; i < w * temp_height; ++i) {
			int j = i / 8;
			int bit = i % 8;
			char byte = static_cast<char>(temp[j]);
			image_data[i] = (!!(byte & (0x1 << (8-bit)))) * 255;
		}

		delete[] temp;
	}

	fclose(fp);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	free(row_pointers);

	return tex;
}

gli::texture2d SurfaceFactory::load_jpeg(const boost::filesystem::path &path, bool srgb) {
	int row_stride;
	std::string content;

	std::ifstream fs(path.string(), std::ios::in);
	if (!fs.good()) {
		using namespace Attributes;
		ste_log_error() << Text::AttributedString("Can't open JPEG ") + i(path.string()) + ": " + std::strerror(errno) << std::endl;
		return gli::texture2d();
	}

	content = std::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
	fs.close();

	unsigned char *data = reinterpret_cast<unsigned char*>(&content[0]);

	if (content.size() == 0) {
		ste_log_error() << "Can't open JPEG: " << path;
		return gli::texture2d();
	}

	auto tj = tjInitDecompress();
	if (tj == nullptr) {
		ste_log_error() << path << ": libturbojpeg signaled error.";
		return gli::texture2d();
	}

	int w, h, chro_sub_smpl, color_space;
	tjDecompressHeader3(tj, data, content.size(), &w, &h, &chro_sub_smpl, &color_space);

	// Read colorspace and components
	gli::format gli_format;
	int comp = 0;
	switch (color_space) {
	case TJCS_GRAY:			gli_format = srgb ? gli::format::FORMAT_R8_SRGB_PACK8 : gli::format::FORMAT_R8_UNORM_PACK8; comp = 1; break;
	default:				gli_format = srgb ? gli::format::FORMAT_RGB8_SRGB_PACK8 : gli::format::FORMAT_RGB8_UNORM_PACK8; comp = 3; break;
	}

	// Create surface
	row_stride = w * comp;

	if (3 - ((row_stride - 1) % 4))
		ste_log_warn() << path << " image not 4byte aligned!";
	auto corrected_stride = row_stride + 3 - ((row_stride - 1) % 4);
	auto w0 = corrected_stride / comp + !!(corrected_stride%comp);
	gli::texture2d tex(gli_format, { w0, h }, 1);
	unsigned char *image_data = reinterpret_cast<unsigned char*>(tex.data());
	int level0_size = tex[0].size();
	if (image_data == nullptr || level0_size < h * row_stride) {
		ste_log_error() << path << " could not allocate memory for JPEG image data or format mismatch" << std::endl;
		tjDestroy(tj);
		return gli::texture2d();
	}

	if (tjDecompress2(tj,
					  data,
					  content.size(),
					  image_data,
					  w,
					  w0 * comp,
					  h,
					  TJPF_RGB,
					  TJFLAG_BOTTOMUP) != 0) {
		const char *err = tjGetErrorStr();
		ste_log_error() << path << " libturbojpeg could not decompress JPEG image: " << (err ? err : "") << std::endl;
		tjDestroy(tj);
		return gli::texture2d();
	}

	tjDestroy(tj);

	return tex;
}
