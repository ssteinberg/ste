// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <ios>

#define BOOST_FILESYSTEM_NO_DEPRECATED 
#include <boost/filesystem.hpp>

#include "task.h"
#include "Texture2D.h"

#include "Log.h"
#include "AttributedString.h"

namespace StE {
namespace Resource {

class SurfaceIO {
private:
	static gli::texture2d load_png(const boost::filesystem::path &file_name, bool srgb);
	static gli::texture2d load_tga(const boost::filesystem::path &file_name, bool srgb);
	static gli::texture2d load_jpeg(const boost::filesystem::path &file_name, bool srgb);
	static bool write_png(const boost::filesystem::path &file_name, const char *image_data, int components, int width, int height);

	~SurfaceIO() {}

public:
	static task<bool> write_surface_2d_task(const gli::texture2d &surface, const boost::filesystem::path &path) {
		return [=](optional<task_scheduler*> sched) -> bool {
			int components;
			if (surface.format() == gli::format::FORMAT_R8_UNORM_PACK8)			components = 1;
			else if (surface.format() == gli::format::FORMAT_RGB8_UNORM_PACK8)	components = 3;
			else if (surface.format() == gli::format::FORMAT_RGBA8_UNORM_PACK8)	components = 4;
			else {
				ste_log_error() << "Can't write surface file: " << path.string() << std::endl;
				return false;
			}
			return write_png(path, reinterpret_cast<const char*>(surface.data()), components, surface.extent().x, surface.extent().y);
		};
	}

	static task<gli::texture2d> load_surface_2d_task(const boost::filesystem::path &path, bool srgb) {
		return [=](optional<task_scheduler*> sched) -> gli::texture2d {
			unsigned char magic[4] = { 0, 0, 0, 0 };

			// Check image format
			{
				try {
					std::ifstream f;
					f.exceptions(f.exceptions() | std::ios::failbit);

					f.open(path.string(), std::ios::binary | std::ios::in);
					if (!f.read(reinterpret_cast<char*>(magic), 4)) {
						ste_log_error() << "Can't read surface file: " << path.string() << std::endl;
						return gli::texture2d();
					}
				}
				catch (const std::ios_base::failure& e) {
					ste_log_error() << "Unknown failure opening file: " << path.string() << " - " << e.what() << " " << std::strerror(errno) << std::endl;
					return gli::texture2d();
				}
			}

			bool generate_mipmaps = true;
			
			using namespace StE::Text;
			using namespace Attributes;

			// Load image
			if (magic[0] == 'D' && magic[1] == 'D' && magic[2] == 'S' && magic[3] == ' ') {
				// DDS
				// Directly construct GLI surface
				auto texture = gli::texture2d(gli::load_dds(path.string()));
				if (texture.empty())
					ste_log_error() << "Can't parse DDS surface: " << path;
				else
					ste_log() << AttributedString("Loaded DDS surface \"") + i(path.string()) + "\" (" + std::to_string(texture.extent().x) + " X " + std::to_string(texture.extent().y) + ") successfully." << std::endl;
				return texture;
			}

			if (magic[0] == 0xff && magic[1] == 0xd8) {
				// JPEG
				auto texture = load_jpeg(path, srgb);
				if (texture.empty())
					ste_log_error() << "Can't parse JPEG surface: " << path;
				else
					ste_log() << AttributedString("Loaded JPEG surface \"") + i(path.string()) + "\" (" + std::to_string(texture.extent().x) + " X " + std::to_string(texture.extent().y) + ") successfully." << std::endl;
				return texture;
			}
			else if (magic[0] == 0x89 && magic[1] == 0x50 && magic[2] == 0x4e && magic[3] == 0x47) {
				// PNG
				auto texture = load_png(path, srgb);
				if (texture.empty())
					ste_log_error() << "Can't parse PNG surface: " << path;
				else
					ste_log() << AttributedString("Loaded PNG surface \"") + i(path.string()) + "\" (" + std::to_string(texture.extent().x) + " X " + std::to_string(texture.extent().y) + ") successfully." << std::endl;
				return texture;
			}
			else if (magic[0] == 0 && magic[1] == 0 && magic[3] == 0) {
				// TGA?
				auto texture = load_tga(path, srgb);
				if (texture.empty())
					ste_log_error() << "Can't parse TGA surface: " << path.string() << std::endl;
				else
					ste_log() << AttributedString("Loaded TGA surface \"") + i(path.string()) + "\" (" + std::to_string(texture.extent().x) + " X " + std::to_string(texture.extent().y) + ") successfully." << std::endl;
				return texture;
			}
			else {
				ste_log_error() << red(AttributedString("Incompatible surface format: \"")) + i(path.string()) + "\"." << std::endl;
				return gli::texture2d();
			}
		};
	}

	static auto load_texture_2d_task(const boost::filesystem::path &path, bool srgb) {
		return task<gli::texture2d>([=](optional<task_scheduler*> sched) {
			return load_surface_2d_task(path, srgb)(sched);
		}).then_on_main_thread([](optional<task_scheduler*> sched, const gli::texture2d &surface) {
			return std::make_unique<LLR::Texture2D>(surface, surface.levels() == 1);
		});
	}
};

}
}
