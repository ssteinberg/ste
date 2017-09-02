//  StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <opaque_surface.hpp>
#include <surface.hpp>
#include <surface_convert.hpp>
#include <surface_copy.hpp>

#include <lib/string.hpp>
#include <fstream>
#include <lib/unique_ptr.hpp>
#include <ios>

#include <filesystem>

#include <log.hpp>
#include <attributed_string.hpp>

#include <surface_factory_exceptions.hpp>

#include <optional.hpp>

namespace ste {
namespace resource {

class surface_io {
private:
	static opaque_surface<2> load_png_2d(const std::experimental::filesystem::path &file_name, bool srgb);
	static opaque_surface<2> load_tga_2d(const std::experimental::filesystem::path &file_name, bool srgb);
	static opaque_surface<2> load_jpeg_2d(const std::experimental::filesystem::path &file_name, bool srgb);
	static opaque_surface<1> load_dds_1d(const std::experimental::filesystem::path &file_name);
	static opaque_surface<2> load_dds_2d(const std::experimental::filesystem::path &file_name);
	static opaque_surface<3> load_dds_3d(const std::experimental::filesystem::path &file_name);
	static opaque_surface<1> load_ktx_1d(const std::experimental::filesystem::path &file_name);
	static opaque_surface<2> load_ktx_2d(const std::experimental::filesystem::path &file_name);
	static opaque_surface<3> load_ktx_3d(const std::experimental::filesystem::path &file_name);
	static void write_png_2d(const std::experimental::filesystem::path &file_name, const std::uint8_t *image_data, int components, int width, int height);
	static void write_tga_2d(const std::experimental::filesystem::path &file_name, const std::uint8_t *image_data, int components, int width, int height);
	static void write_jpeg_2d(const std::experimental::filesystem::path &file_name, const std::uint8_t *image_data, int components, int width, int height);
	static void write_dds(const std::experimental::filesystem::path &file_name, const std::uint8_t *image_data, std::size_t bytes, gl::format format, gl::image_type image_type, const glm::u32vec3 &extent, std::uint32_t levels, std::uint32_t layers);

	~surface_io() noexcept {}

public:
	enum class surface_write_file_format {
		png,
		tga,
		jpeg,
		dds
	};

public:
	/**
	 *	@brief	Writes 2D surface to file. Converts the surface to a 8-bit srgb format if needed.
	 *
	 *	@param	surface			Input surface
	 *	@param	path			Filesystem path of output
	 *	@param	file_format		Selects desired output file format
	 */
	template <gl::format format>
	static void write_image_2d(const surface_2d<format> &surface,
							   const std::experimental::filesystem::path &path,
							   const surface_write_file_format &file_format = surface_write_file_format::png) {
		static constexpr auto elements = gl::format_traits<format>::elements;
		static constexpr auto write_format = elements == 1 ? gl::format::r8_srgb : (elements == 3 ? gl::format::r8g8b8_srgb : gl::format::r8g8b8a8_srgb);

		// Select writer
		void(*writer)(const std::experimental::filesystem::path &, const std::uint8_t *, int, int, int);
		switch (file_format) {
		case surface_write_file_format::png:
			writer = write_png_2d;
			if (elements != 1 && elements != 3 && elements != 4) {
				using namespace text::attributes;
				ste_log_error() << "Can not write image using selected file format: " << i(lib::to_string(path.string())) << std::endl;
				throw surface_unsupported_format_error("Can not write image using selected file format");
			}
			break;
		case surface_write_file_format::tga:
			writer = write_tga_2d;
			if (elements != 1 && elements != 3 && elements != 4) {
				using namespace text::attributes;
				ste_log_error() << "Can not write image using selected file format: " << i(lib::to_string(path.string())) << std::endl;
				throw surface_unsupported_format_error("Can not write image using selected file format");
			}
			break;
		case surface_write_file_format::jpeg:
			writer = write_jpeg_2d;
			if (elements != 1 && elements != 3) {
				using namespace text::attributes;
				ste_log_error() << "Can not write image using selected file format: " << i(lib::to_string(path.string())) << std::endl;
				throw surface_unsupported_format_error("Can not write image using selected file format");
			}
			break;
		default: {
			using namespace text::attributes;
			ste_log_error() << "Can not write image using selected file format: " << i(lib::to_string(path.string())) << std::endl;
			throw surface_unsupported_format_error("Can not write image using selected file format");
		}
		}

		// Convert (if needed) and write
		if constexpr (write_format != format) {
			auto src = surface_convert::convert_2d<write_format>(surface_copy::copy_2d(surface));
			writer(path, 
				   reinterpret_cast<const std::uint8_t *>(src.data()), 
				   elements, 
				   src.extent().x, src.extent().y);
		}
		else {
			writer(path, 
				   reinterpret_cast<const std::uint8_t *>(surface.data()), 
				   elements, 
				   surface.extent().x, surface.extent().y);
		}
	}

	/**
	*	@brief	Writes surface to file.
	*
	*	@param	surface			Input surface
	*	@param	path			Filesystem path of output
	*	@param	file_format		Selects desired output file format
	*/
	template<gl::format format, gl::image_type image_type>
	static void write_surface(const surface_generic<format, image_type> &surface,
							  const std::experimental::filesystem::path &path,
							  const surface_write_file_format &file_format = surface_write_file_format::dds) {
		static constexpr auto dimensions = gl::image_dimensions_v<image_type>;

		// Select writer
		void(*writer)(const std::experimental::filesystem::path &, const std::uint8_t*, std::size_t, gl::format, gl::image_type, const glm::u32vec3&, std::uint32_t, std::uint32_t);
		switch (file_format) {
		case surface_write_file_format::dds:
			writer = write_dds;
			break;
		default: {
			using namespace text::attributes;
			ste_log_error() << "Can not write image using selected file format: " << i(lib::to_string(path.string())) << std::endl;
			throw surface_unsupported_format_error("Can not write image using selected file format");
		}
		}

		// Write out
		glm::u32vec3 extent = { 1,1,1 };
		if constexpr (dimensions>0) extent.x = surface.extent().x;
		if constexpr (dimensions>1) extent.y = surface.extent().y;
		if constexpr (dimensions>2) extent.z = surface.extent().z;

		writer(path, 
			   reinterpret_cast<const std::uint8_t *>(surface.data()), 
			   surface.bytes(), 
			   format, 
			   image_type, 
			   extent, 
			   static_cast<std::uint32_t>(surface.levels()),
			   static_cast<std::uint32_t>(surface.layers()));
	}

	/**
	 *	@brief	Loads 2D surface from file
	 *	
	 *	@param	path	Filesystem path of input file
	 *	@param	srgb	Use sRGB non-linear color space
	 */
	static auto load_surface_2d(const std::experimental::filesystem::path &path, bool srgb = false) {
		unsigned char magic[4] = { 0, 0, 0, 0 };

		// Check image format
		{
			try {
				std::ifstream f;
				f.exceptions(f.exceptions() | std::ios::failbit);

				f.open(path.string(), std::ios::binary | std::ios::in);
				if (!f) {
					ste_log_error() << "Can't open surface file: " << path.string() << std::endl;
					throw resource_io_error("Resource IO error");
				}
				if (!f.read(reinterpret_cast<char*>(magic), 4)) {
					ste_log_error() << "Can't read surface file: " << path.string() << std::endl;
					throw resource_io_error("Surface IO error");
				}
			}
			catch (const std::ios_base::failure& e) {
				ste_log_error() << "Unknown failure opening file: " << path.string() << " - " << e.what() << " " << std::strerror(errno) << std::endl;
				throw resource_io_error("Resource IO error");
			}
		}

		// Load image
		{
			using namespace text;
			using namespace attributes;

			if (magic[0] == 'D' && magic[1] == 'D' && magic[2] == 'S' && magic[3] == ' ') {
				// DDS
				auto texture = load_dds_2d(path.string());

				ste_log() << attributed_string("Loaded DDS surface \"") + i(lib::to_string(path.string())) + "\" (" + lib::to_string(texture.extent().x) + " X " + lib::to_string(texture.extent().y) + ") successfully." << std::endl;
				return texture;
			}
			if (magic[0] == 0xAB && magic[1] == 0x4B && magic[2] == 0x54 && magic[3] == 0x58) {
				// KTX
				auto texture = load_ktx_2d(path.string());

				ste_log() << attributed_string("Loaded KTX surface \"") + i(lib::to_string(path.string())) + "\" (" + lib::to_string(texture.extent().x) + " X " + lib::to_string(texture.extent().y) + ") successfully." << std::endl;
				return texture;
			}
			if (magic[0] == 0xff && magic[1] == 0xd8) {
				// JPEG
				auto texture = load_jpeg_2d(path, srgb);

				ste_log() << attributed_string("Loaded JPEG surface \"") + i(lib::to_string(path.string())) + "\" (" + lib::to_string(texture.extent().x) + " X " + lib::to_string(texture.extent().y) + ") successfully." << std::endl;
				return texture;
			}
			if (magic[0] == 0x89 && magic[1] == 0x50 && magic[2] == 0x4e && magic[3] == 0x47) {
				// PNG
				auto texture = load_png_2d(path, srgb);

				ste_log() << attributed_string("Loaded PNG surface \"") + i(lib::to_string(path.string())) + "\" (" + lib::to_string(texture.extent().x) + " X " + lib::to_string(texture.extent().y) + ") successfully." << std::endl;
				return texture;
			}
			if (magic[0] == 0 && magic[1] == 0 && magic[3] == 0) {
				// TGA?
				auto texture = load_tga_2d(path, srgb);

				ste_log() << attributed_string("Loaded TGA surface \"") + i(lib::to_string(path.string())) + "\" (" + lib::to_string(texture.extent().x) + " X " + lib::to_string(texture.extent().y) + ") successfully." << std::endl;
				return texture;
			}

			ste_log_error() << red(attributed_string("Incompatible surface format: \"")) + i(lib::to_string(path.string())) + "\"." << std::endl;
			throw surface_unsupported_format_error("Incompatible surface format");
		}
	}
};

}
}
