// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <string>
#include <gli/gli.hpp>
#include <memory>

namespace StE {
namespace LLR {

class Texture2D {
private:
	GLuint id;
	int levels;
	short w, h;
	gli::format format;

	static gli::gl GL;
	static int calculate_mipmap_max_level(int w, int h);

public:
	Texture2D(Texture2D &&t) = default;
	Texture2D(const Texture2D &t) = delete;

	Texture2D &operator=(Texture2D &&t) = default;
	Texture2D &operator=(const Texture2D &t) = delete;

	Texture2D(gli::format format, int w, int h, int levels) { allocate_tex_storage(w, h, format, levels); }
	Texture2D(const gli::texture2D &t, bool generate_mipmaps = false) : Texture2D(t.format(), t.dimensions().x, t.dimensions().y, generate_mipmaps ? calculate_mipmap_max_level(t.dimensions().x, t.dimensions().y) + 1 : t.levels()) {
		upload(t, generate_mipmaps); 
	}
	virtual ~Texture2D() { if (this->is_valid()) glDeleteTextures(1, &id); }

	// Reupload texture data. Surface must match texture's format.
	bool upload(const gli::texture2D &t, bool generate_mipmaps = false);

	void generate_mipmaps() {
		glEnable(GL_TEXTURE_2D);
		bind(0);
		glGenerateMipmap(GL_TEXTURE_2D); 
	}

	void bind(int sampler) const { assert(is_valid()); glActiveTexture(GL_TEXTURE0 + sampler); glBindTexture(GL_TEXTURE_2D, id); }
	static void unbind(int sampler) { glActiveTexture(GL_TEXTURE0 + sampler); glBindTexture(GL_TEXTURE_2D, 0); }

	void set_mag_filter(GLenum filter) { glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter); }
	void set_min_filter(GLenum filter) { glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter); }

	bool is_valid() const { return id != 0; }
	int get_w() const { return w; }
	int get_h() const { return h; }
	int get_levels() const { return levels; }
	gli::format get_format() const { return format; }
	GLuint get_texture_id() const { return id; }

protected:
	bool allocate_tex_storage(int w, int h, gli::format format, int levels);
};

}
}
