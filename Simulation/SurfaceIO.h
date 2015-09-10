// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <string>
#include <fstream>
#include <memory>

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
	static bool write_surface_2d(const gli::texture2D &surface, const std::string &path) {
		int components;
		if (surface.format() == gli::format::FORMAT_R8_UNORM)			components = 1;
		else if (surface.format() == gli::format::FORMAT_RGB8_UNORM)	components = 3;
		else if (surface.format() == gli::format::FORMAT_RGBA8_UNORM)	components = 4;
		else {
			ste_log_error() << "Can't write texture file: " << path;
			return false;
		}
		return write_png(path, reinterpret_cast<const char*>(surface.data()), components, surface.dimensions().x, surface.dimensions().y);
	}

	static gli::texture2D load_surface_2d(const std::string &path) {
		unsigned char magic[4] = { 0, 0, 0, 0 };

		// Check image format
		{
			std::ifstream stream(path.data(), std::ios::binary | std::ios::in);
			if (!stream.read(reinterpret_cast<char*>(magic), 4)) {
				ste_log_error() << "Can't read texture file: " << path;
				return gli::texture2D();
			}
		}

		bool generate_mipmaps = true;

		// Load image
		if (magic[0] == 'D' && magic[1] == 'D' && magic[2] == 'S' && magic[3] == ' ') {
			// DDS
			// Directly construct GLI surface
			auto texture = gli::texture2D(gli::load_dds(path));
			if (texture.empty())
				ste_log_error() << "Can't parse DDS texture: " << path;
			return texture;
		}

		if (magic[0] == 0xff && magic[1] == 0xd8) {
			// JPEG
			auto texture = load_jpeg(path);
			if (texture.empty())
				ste_log_error() << "Can't parse JPEG texture: " << path;
			return texture;
		}
		else if (magic[0] == 0x89 && magic[1] == 0x50 && magic[2] == 0x4e && magic[3] == 0x47) {
			// PNG
			auto texture = load_png(path);
			if (texture.empty())
				ste_log_error() << "Can't parse PNG texture: " << path;
			return texture;
		}
		else {
			ste_log_error() << "Incompatible texture format: " << path;
			return gli::texture2D();
		}
	}

	static std::unique_ptr<LLR::Texture2D> load_2d(const std::string &path) {
		auto surface = load_surface_2d(path);
		if (surface.empty())
			return nullptr;
		return std::unique_ptr<LLR::Texture2D>(new LLR::Texture2D(surface, surface.levels()==1));
	}
};

}
}
