// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <lib/string.hpp>
#include <fstream>
#include <lib/unique_ptr.hpp>
#include <ios>

#include <filesystem>

#include <task_future.hpp>
#include <task_scheduler.hpp>

#include <Log.hpp>
#include <attributed_string.hpp>

#include <surface_factory_exceptions.hpp>

#include <gli/load_dds.hpp>

namespace ste {
namespace resource {

class surface_io {
private:
	static gli::texture2d load_png(const std::experimental::filesystem::path &file_name, bool srgb);
	static gli::texture2d load_tga(const std::experimental::filesystem::path &file_name, bool srgb);
	static gli::texture2d load_jpeg(const std::experimental::filesystem::path &file_name, bool srgb);
	static void write_png(const std::experimental::filesystem::path &file_name, const char *image_data, int components, int width, int height);

	~surface_io() {}

public:
	static void write_surface_2d(const gli::texture2d &surface, const std::experimental::filesystem::path &path) {
		int components;
		if (surface.format() == gli::format::FORMAT_R8_UNORM_PACK8)			components = 1;
		else if (surface.format() == gli::format::FORMAT_RGB8_UNORM_PACK8)	components = 3;
		else if (surface.format() == gli::format::FORMAT_RGBA8_UNORM_PACK8)	components = 4;
		else {
			ste_log_error() << "Can't write surface file: " << path.string() << std::endl;
			throw surface_unsupported_format_error("Unsupported surface format");
		}

		write_png(path, reinterpret_cast<const char*>(surface.data()), components, surface.extent().x, surface.extent().y);
	}
	static auto write_surface_2d_async(task_scheduler &sched, const gli::texture2d &surface, const std::experimental::filesystem::path &path) {
		return sched.schedule_now([&]() {
			write_surface_2d(surface, path);
		});
	}

	static auto load_surface_2d(const std::experimental::filesystem::path &path, bool srgb) {
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

		using namespace ste::text;
		using namespace Attributes;

		// Load image
		if (magic[0] == 'D' && magic[1] == 'D' && magic[2] == 'S' && magic[3] == ' ') {
			// DDS
			// Directly construct GLI surface
			auto texture = gli::texture2d(gli::load_dds(path.string()));
			if (texture.empty()) {
				ste_log_error() << "Can't parse DDS surface: " << path;
				throw surface_error("Parsing DDS surface failed");
			}

			ste_log() << attributed_string("Loaded DDS surface \"") + i(lib::to_string(path.string())) + "\" (" + lib::to_string(texture.extent().x) + " X " + lib::to_string(texture.extent().y) + ") successfully." << std::endl;
			return texture;
		}

		if (magic[0] == 0xff && magic[1] == 0xd8) {
			// JPEG
			auto texture = load_jpeg(path, srgb);
			if (texture.empty()) {
				ste_log_error() << "Can't parse JPEG surface: " << path;
				throw surface_error("Parsing JPEG surface failed");
			}

			ste_log() << attributed_string("Loaded JPEG surface \"") + i(lib::to_string(path.string())) + "\" (" + lib::to_string(texture.extent().x) + " X " + lib::to_string(texture.extent().y) + ") successfully." << std::endl;
			return texture;
		}
		else if (magic[0] == 0x89 && magic[1] == 0x50 && magic[2] == 0x4e && magic[3] == 0x47) {
			// PNG
			auto texture = load_png(path, srgb);
			if (texture.empty()) {
				ste_log_error() << "Can't parse PNG surface: " << path;
				throw surface_error("Parsing PNG surface failed");
			}

			ste_log() << attributed_string("Loaded PNG surface \"") + i(lib::to_string(path.string())) + "\" (" + lib::to_string(texture.extent().x) + " X " + lib::to_string(texture.extent().y) + ") successfully." << std::endl;
			return texture;
		}
		else if (magic[0] == 0 && magic[1] == 0 && magic[3] == 0) {
			// TGA?
			auto texture = load_tga(path, srgb);
			if (texture.empty()) {
				ste_log_error() << "Can't parse TGA surface: " << path.string() << std::endl;
				throw surface_error("Parsing TGA surface failed");
			}

			ste_log() << attributed_string("Loaded TGA surface \"") + i(lib::to_string(path.string())) + "\" (" + lib::to_string(texture.extent().x) + " X " + lib::to_string(texture.extent().y) + ") successfully." << std::endl;
			return texture;
		}
		else {
			ste_log_error() << red(attributed_string("Incompatible surface format: \"")) + i(lib::to_string(path.string())) + "\"." << std::endl;
			throw surface_unsupported_format_error("Incompatible surface format");
		}
	}

	static auto load_surface_2d_async(task_scheduler &sched, const std::experimental::filesystem::path &path, bool srgb) {
		return sched.schedule_now([&]() {
			return lib::allocate_unique<gli::texture2d>(load_surface_2d(path, srgb));
		});
	}
};

}
}
