
#include "stdafx.h"
#include "SurfaceIO.h"
#include "Log.h"

#include <png/png.h>
#include <libjpeg/jpeglib.h>
#include <libtga/tga.h>

using namespace StE::Resource;

bool SurfaceIO::write_png(const std::string &file_name, const char *image_data, int components, int width, int height) {
	if (components != 1 && components != 3 && components != 4) {
		ste_log_error() << file_name << " can't write " << components << " channel PNG.";
		return false;
	}

	FILE *fp = fopen(file_name.data(), "wb");
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
		row_pointers[height - 1 - i] = (png_byte*)reinterpret_cast<const png_byte*>(image_data + i * width * components);

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

gli::texture2D SurfaceIO::load_tga(const std::string &file_name, bool srgb) {
	TGA *tga;

	tga = TGAOpen(const_cast<char*>(file_name.c_str()), "rb");

	TGAReadHeader(tga);
	if (tga->last != TGA_OK) {
		TGAClose(tga);
		ste_log_error() << file_name << " is not a valid 24-bit TGA" << std::endl;
		return gli::texture2D();
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
			format = srgb ? gli::format::FORMAT_R8_SRGB : gli::format::FORMAT_R8_UNORM;
			components = 1;
			break;
		}
		else if (tga->hdr.depth == 24) {
			format = srgb ? gli::format::FORMAT_BGR8_SRGB : gli::format::FORMAT_BGR8_UNORM;
			components = 3;
			break;
		}
		else if (tga->hdr.depth == 32) {
			format = srgb ? gli::format::FORMAT_BGRA8_SRGB : gli::format::FORMAT_BGRA8_UNORM;
			components = 4;
			break;
		}
	default:
		TGAClose(tga);
		ste_log_error() << file_name << " Unsupported libtga depth (" << tga->hdr.depth << ") and image type (" << tga->hdr.img_t << ") combination" << std::endl;
		return gli::texture2D();
	}

	unsigned rowbytes = w * components;
	int rowsize = rowbytes;
	if (3 - ((rowbytes - 1) % 4))
		ste_log_warn() << file_name << " image row not 4byte aligned!";
	rowbytes += 3 - ((rowbytes - 1) % 4);

	w = rowbytes / components + !!(rowbytes%components);
	gli::texture2D tex(1, format, { w, h });
	tbyte *image_data = reinterpret_cast<tbyte*>(tex.data());
	auto level0_size = tex[0].size();
	if (image_data == nullptr || level0_size < rowbytes*h) {
		TGAClose(tga);
		ste_log_error() << file_name << " could not allocate memory for TGA image data or format mismatch";
		return gli::texture2D();
	}

	TGAReadScanlines(tga, image_data, 0, h, TGA_BGR);

	TGAClose(tga);

	return tex;
}

gli::texture2D SurfaceIO::load_png(const std::string &file_name, bool srgb) {
	png_byte header[8];

	FILE *fp = fopen(file_name.data(), "rb");
	if (fp == 0) {
		perror(file_name.data());
		return gli::texture2D();
	}

	// read the header
	fread(header, 1, 8, fp);

	if (png_sig_cmp(header, 0, 8)) {
		ste_log_error() << file_name << " is not a PNG";
		fclose(fp);
		return gli::texture2D();
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		ste_log_error() << file_name << " png_create_read_struct returned 0";
		fclose(fp);
		return gli::texture2D();
	}

	// create png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		ste_log_error() << file_name << " png_create_info_struct returned 0";
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(fp);
		return gli::texture2D();
	}

	// create png info struct
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		ste_log_error() << file_name << " png_create_info_struct returned 0";
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(fp);
		return gli::texture2D();
	}

	// the code in this if statement gets called if libpng encounters an error
	if (setjmp(png_jmpbuf(png_ptr))) {
		ste_log_error() << file_name << " error from libpng";
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return gli::texture2D();
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
		return gli::texture2D();
	}

	gli::format format;
	int components;
	switch (color_type) {
	case PNG_COLOR_TYPE_GRAY:
		format = srgb ? gli::format::FORMAT_R8_SRGB : gli::format::FORMAT_R8_UNORM;
		components = 1;
		break;
	case PNG_COLOR_TYPE_RGB:
		format = srgb ? gli::format::FORMAT_RGB8_SRGB : gli::format::FORMAT_RGB8_UNORM;
		components = 3;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		format = srgb ? gli::format::FORMAT_RGBA8_SRGB : gli::format::FORMAT_RGBA8_UNORM;
		components = 4;
		break;
	default:
		ste_log_error() << file_name << " Unknown libpng color type " << color_type;
		fclose(fp);
		return gli::texture2D();
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
	gli::texture2D tex(1, format, { w, temp_height });
	char *image_data = reinterpret_cast<char*>(tex.data());
	auto level0_size = tex[0].size();
	if (image_data == nullptr || level0_size < rowbytes*temp_height) {
		ste_log_error() << file_name << " could not allocate memory for PNG image data or format mismatch";
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return gli::texture2D();
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

struct libjpeg_error_mgr {
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};
typedef struct libjpeg_error_mgr * my_error_ptr;

METHODDEF(void) jpeg_error_exit(j_common_ptr cinfo) {
	my_error_ptr myerr = (my_error_ptr)cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

gli::texture2D SurfaceIO::load_jpeg(const std::string &path, bool srgb) {
	const char * filename = path.data();

	struct jpeg_decompress_struct cinfo;
	struct libjpeg_error_mgr jerr;

	FILE * infile;
	int row_stride;

	if ((infile = fopen(filename, "rb")) == NULL) {
		ste_log_error() << "Can't open JPEG: " << path;
		return gli::texture2D();
	}

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpeg_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		ste_log_error() << path << ": libjpeg signaled error.";
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return gli::texture2D();
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */

	(void)jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.txt for more info.
	*/

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	* jpeg_read_header(), so we do nothing here.
	*/

	/* Step 5: Start decompressor */

	(void)jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	// Read colorspace and components
	gli::format gli_format;
	bool sanity = true;
	switch (cinfo.out_color_space) {
	case JCS_GRAYSCALE:		gli_format = srgb ? gli::format::FORMAT_R8_SRGB : gli::format::FORMAT_R8_UNORM; sanity = cinfo.output_components == 1; break;
	case JCS_EXT_RGB:
	case JCS_RGB:			gli_format = srgb ? gli::format::FORMAT_RGB8_SRGB : gli::format::FORMAT_RGB8_UNORM; sanity = cinfo.output_components == 3; break;
	case JCS_EXT_RGBA:		gli_format = srgb ? gli::format::FORMAT_RGBA8_SRGB : gli::format::FORMAT_RGBA8_UNORM; sanity = cinfo.output_components == 4; break;
	default:				sanity = false;
	}
	if (!sanity) {
		ste_log_error() << path << " Unsupported JPEG components count (" << cinfo.output_components << ") and colorspace (" << cinfo.out_color_space << ") combination";
		(void)jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return gli::texture2D();
	}

	// Create surface
	row_stride = cinfo.output_width * cinfo.output_components;

	if (3 - ((row_stride - 1) % 4))
		ste_log_warn() << path << " image not 4byte aligned!";
	auto corrected_stride = row_stride + 3 - ((row_stride - 1) % 4);
	auto w = corrected_stride / cinfo.output_components + !!(corrected_stride%cinfo.output_components);
	gli::texture2D tex(1, gli_format, { w, cinfo.output_height });
	char *image_data = reinterpret_cast<char*>(tex.data());
	auto level0_size = tex[0].size();
	if (image_data == nullptr || level0_size < cinfo.output_height * row_stride) {
		ste_log_error() << path << " could not allocate memory for JPEG image data or format mismatch";
		(void)jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return gli::texture2D();
	}

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */
	/* Create row_pointers to invert the image */
	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/

	unsigned char ** row_pointers = (unsigned char **)malloc(cinfo.output_height * sizeof(unsigned char *));
	if (row_pointers == NULL) {
		ste_log_error() << path << " could not allocate memory for JPEG row pointers";
		(void)jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return gli::texture2D();
	}
	// set the individual row_pointers to point at the correct offsets of image_data
	for (unsigned int i = 0; i < cinfo.output_height; i++)
		row_pointers[cinfo.output_height - 1 - i] = reinterpret_cast<unsigned char*>(image_data + i * w * cinfo.output_components);

	// Read as many lines per call as possible
	unsigned lines_read = 0;
	while (cinfo.output_scanline < cinfo.output_height) {
		lines_read += jpeg_read_scanlines(&cinfo, row_pointers + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);
	}
	if (lines_read < cinfo.output_height)
		ste_log_warn() << path << " JPEG decoding ended prematurely. Read " << lines_read << "/" << cinfo.output_height << " scan lines";

	/* Step 7: Finish decompression */

	(void)jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* Step 8: Release JPEG decompression object */
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	/* At this point you may want to check to see whether any corrupt-data
	* warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	*/

	return tex;
}
