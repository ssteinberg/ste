
#include <stdafx.hpp>
#include <surface_io.hpp>

#include <log.hpp>
#include <attributed_string.hpp>
#include <attrib.hpp>

#include <lib/vector.hpp>

#include <turbojpeg.h>

using namespace ste;
using namespace ste::text;
using namespace ste::resource;

opaque_surface<2> surface_io::load_jpeg(const std::experimental::filesystem::path &path, bool srgb) {
	std::ifstream fs(path.string(), std::ios::in);
	if (!fs) {
		using namespace attributes;
		ste_log_error() << text::attributed_string("Can't open JPEG ") + i(lib::to_string(path.string())) + ": " + std::strerror(errno) << std::endl;
		throw resource_io_error("Could not open file");
	}

	auto content = lib::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
	fs.close();

	unsigned char *data = reinterpret_cast<unsigned char*>(&content[0]);

	if (content.size() == 0) {
		ste_log_error() << "Can't open JPEG: " << path;
		throw resource_io_error("Reading file failed");
	}

	const auto tj = tjInitDecompress();
	if (tj == nullptr) {
		ste_log_error() << path << ": libturbojpeg signaled error.";
		throw surface_error("libjpegturbo error");
	}

	int w, h, chro_sub_smpl, color_space;
	tjDecompressHeader3(tj, data, static_cast<unsigned long>(content.size()), &w, &h, &chro_sub_smpl, &color_space);

	// Read colorspace and components
	gl::format format;
	int comp = 0;
	int pixel_format;
	switch (color_space) {
	case TJCS_GRAY:
		format = srgb ? gl::format::r8_srgb : gl::format::r8_unorm; 
		comp = 1;
		pixel_format = TJPF_GRAY;
		break;
	default:
		format = srgb ? gl::format::r8g8b8_srgb : gl::format::r8g8b8_unorm;
		comp = 3;
		pixel_format = TJPF_RGB;
		break;
	}

	// Create surface
	const auto row_stride = w * comp;

	if (3 - ((row_stride - 1) % 4))
		ste_log_warn() << path << " image not 4byte aligned!";
	const auto corrected_stride = row_stride + 3 - ((row_stride - 1) % 4);
	const auto w0 = corrected_stride / comp + !!(corrected_stride%comp);
	const auto level0_size = corrected_stride * h;

	lib::vector<std::uint8_t> image_data;
	image_data.resize(level0_size);

	if (tjDecompress2(tj,
					  data,
					  static_cast<unsigned long>(content.size()),
	                  reinterpret_cast<unsigned char*>(image_data.data()),
					  w,
					  w0 * comp,
					  h,
					  pixel_format,
					  TJFLAG_BOTTOMUP) != 0) {
		const char *err = tjGetErrorStr();
		ste_log_error() << path << " libturbojpeg could not decompress JPEG image: " << (err ? err : "") << std::endl;
		tjDestroy(tj);
		throw surface_error("libturbojpeg could not decompress JPEG image");
	}

	tjDestroy(tj);

	opaque_surface<2> tex(format, gl::image_type::image_2d, { w0, h }, 1, 1, std::move(image_data));
	return tex;
}

void surface_io::write_jpeg(const std::experimental::filesystem::path &file_name, const std::uint8_t *image_data, int components, int width, int height) {
	if (components != 1 && components != 3) {
		ste_log_error() << file_name << " can't write " << components << " channel JPEG.";
		throw surface_unsupported_format_error("Unsupported JPEG component count");
	}

	FILE *fp = fopen(file_name.string().data(), "wb");
	if (!fp) {
		ste_log_error() << file_name << " can't be opened for writing";
		throw resource_io_error("Opening output file failed");
	}

	const auto tj = tjInitCompress();
	if (tj == nullptr) {
		ste_log_error() << "libturbojpeg signaled error.";
		throw surface_error("libjpegturbo error");
	}

	// Choose component subsampling and pixel format
	const TJSAMP samp = components == 1 ? TJSAMP_GRAY : TJSAMP_444;
	const int pixel_format = components == 1 ? TJPF_GRAY : TJPF_RGB;

	// Query buffer size and allocate buffer
	const auto size = tjBufSize(width, height, samp);
	lib::vector<std::uint8_t> buffer;
	buffer.resize(size);

	// Compress
	auto dst_buffer = reinterpret_cast<unsigned char*>(buffer.data());
	auto dst_size = static_cast<unsigned long>(buffer.size());
	if (tjCompress2(tj, 
					image_data, 
					width, 
					width * components, 
					height, 
					pixel_format, 
					&dst_buffer, 
					&dst_size, 
					samp, 
					100, 
					TJFLAG_BOTTOMUP | TJFLAG_NOREALLOC) != 0) {
		const char *err = tjGetErrorStr();
		ste_log_error() << "libturbojpeg could not compress JPEG image: " << (err ? err : "") << std::endl;
		tjDestroy(tj);
		throw surface_error("libturbojpeg could not compress JPEG image");
	}

	fwrite(dst_buffer, dst_size, 1, fp);

	fclose(fp);
}
