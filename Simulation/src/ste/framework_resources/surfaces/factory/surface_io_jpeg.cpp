
#include <stdafx.hpp>
#include <surface_io.hpp>

#include <log.hpp>
#include <attributed_string.hpp>
#include <attrib.hpp>

#include <lib/vector.hpp>
#include <lib/unique_ptr.hpp>

#include <turbojpeg.h>

using namespace ste;
using namespace ste::text;
using namespace ste::resource;

opaque_surface<2> surface_io::load_jpeg_2d(const std::experimental::filesystem::path &path, bool srgb) {
	std::ifstream fs(path.string(), std::ios::in | std::ios::binary);
	if (!fs) {
		using namespace attributes;
		ste_log_error() << text::attributed_string("Can't open JPEG ") + i(lib::to_string(path.string())) + ": " + std::strerror(errno) << std::endl;
		throw resource_io_error("Could not open file");
	}

	auto content = lib::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
	fs.close();

	unsigned char *data = reinterpret_cast<unsigned char*>(&content[0]);

	if (content.size() == 0) {
		ste_log_error() << "Can't open JPEG: " << path << std::endl;
		throw resource_io_error("Reading file failed");
	}

	const auto tj = tjInitDecompress();
	if (tj == nullptr) {
		ste_log_error() << path << ": libturbojpeg signaled error." << std::endl;
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

	lib::unique_ptr<std::uint8_t[]> image_data = lib::allocate_unique<std::uint8_t[]>(level0_size);

	if (tjDecompress2(tj,
					  data,
					  static_cast<unsigned long>(content.size()),
	                  image_data.get(),
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

	opaque_surface<2> tex(format, 
						  gl::image_type::image_2d, 
						  { w0, h }, 
						  1, 1, 
						  std::move(image_data), 
						  level0_size);
	return tex;
}

void surface_io::write_jpeg_2d(const std::experimental::filesystem::path &path, const std::uint8_t *image_data, int components, int width, int height) {
	if (components != 1 && components != 3) {
		ste_log_error() << path << " can't write " << components << " channel JPEG." << std::endl;
		throw surface_unsupported_format_error("Unsupported JPEG component count");
	}

	const auto tj = tjInitCompress();
	if (tj == nullptr) {
		ste_log_error() << "libturbojpeg signaled error." << std::endl;
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
					90,		// Quality
					TJFLAG_BOTTOMUP | TJFLAG_NOREALLOC) != 0) {
		const char *err = tjGetErrorStr();
		ste_log_error() << "libturbojpeg could not compress JPEG image: " << (err ? err : "") << std::endl;
		tjDestroy(tj);
		throw surface_error("libturbojpeg could not compress JPEG image");
	}

	// Write out
	{
		std::ofstream fs(path.string(), std::ios::out | std::ios::binary);
		if (!fs) {
			using namespace attributes;
			ste_log_error() << text::attributed_string("Can't open file ") + i(lib::to_string(path.string())) + " for writing: " + std::strerror(errno) << std::endl;
			throw resource_io_error("Could not open file");
		}

		std::copy(buffer.data(), buffer.data() + dst_size, std::ostream_iterator<std::uint8_t>(fs));
	}
}
