// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <string>
#include <fstream>
#include <memory>

#include "task.h"
#include "Texture2D.h"
#include "Log.h"

namespace StE {
namespace Resource {

class SurfaceIO {
private:
	static gli::texture2D load_png(const std::string &file_name);
	static gli::texture2D load_jpeg(const std::string &file_name);
	static bool write_png(const std::string &file_name, const char *image_data, int components, int width, int height);

	~SurfaceIO() {}

public:
	static task<bool> write_surface_2d_task(const gli::texture2D &surface, const std::string &path) {
		return [=](optional<task_scheduler*> sched) -> bool {
			int components;
			if (surface.format() == gli::format::FORMAT_R8_UNORM)			components = 1;
			else if (surface.format() == gli::format::FORMAT_RGB8_UNORM)	components = 3;
			else if (surface.format() == gli::format::FORMAT_RGBA8_UNORM)	components = 4;
			else {
				ste_log_error() << "Can't write surface file: " << path;
				return false;
			}
			return write_png(path, reinterpret_cast<const char*>(surface.data()), components, surface.dimensions().x, surface.dimensions().y);
		};
	}

	static task<gli::texture2D> load_surface_2d_task(const std::string &path) {
		return [=](optional<task_scheduler*> sched) -> gli::texture2D {
			unsigned char magic[4] = { 0, 0, 0, 0 };

			// Check image format
			{
				try {
					std::ifstream f;
					std::ios_base::iostate exceptionMask = f.exceptions() | std::ios::failbit;
					f.exceptions(exceptionMask);

					f.open(path.data(), std::ios::binary | std::ios::in);
					if (!f.read(reinterpret_cast<char*>(magic), 4)) {
						ste_log_error() << "Can't read surface file: " << path << std::endl;
						return gli::texture2D();
					}
				}
				catch (std::ios_base::failure& e) {
					if (e.code() == std::make_error_condition(std::io_errc::stream))
						ste_log_error() << "Stream error reading surface file: " << path << " - " << e.what() << std::endl;
					else
						ste_log_error() << "Unknown failure opening file: " << path << " - " << e.what() << std::endl;
				}
			}

			bool generate_mipmaps = true;

			// Load image
			if (magic[0] == 'D' && magic[1] == 'D' && magic[2] == 'S' && magic[3] == ' ') {
				// DDS
				// Directly construct GLI surface
				ste_log() << "Loading DDS surface " << path;
				auto texture = gli::texture2D(gli::load_dds(path));
				if (texture.empty())
					ste_log_error() << "Can't parse DDS surface: " << path;
				return texture;
			}

			if (magic[0] == 0xff && magic[1] == 0xd8) {
				// JPEG
				ste_log() << "Loading JPEG surface " << path;
				auto texture = load_jpeg(path);
				if (texture.empty())
					ste_log_error() << "Can't parse JPEG surface: " << path;
				return texture;
			}
			else if (magic[0] == 0x89 && magic[1] == 0x50 && magic[2] == 0x4e && magic[3] == 0x47) {
				// PNG
				ste_log() << "Loading PNG surface " << path;
				auto texture = load_png(path);
				if (texture.empty())
					ste_log_error() << "Can't parse PNG surface: " << path;
				return texture;
			}
			else {
				ste_log_error() << "Incompatible surface format: " << path;
				return gli::texture2D();
			}
		};
	}

	static auto load_texture_2d_task(const std::string &path) {
		return task<gli::texture2D>([=](optional<task_scheduler*> sched) {
			return load_surface_2d_task(path)(sched);
		}).then_on_main_thread([](optional<task_scheduler*> sched, const gli::texture2D &surface) {
			 return std::unique_ptr<LLR::Texture2D>(new LLR::Texture2D(surface, surface.levels() == 1));
		});
	}
};

}
}
