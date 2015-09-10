
#include "stdafx.h"
#include "Texture2D.h"
#include "Log.h"

using namespace StE::LLR;

gli::gl Texture2D::GL;

int Texture2D::calculate_mipmap_max_level(int w, int h) {
	int levels;
	for (levels = 0; w >> levels > 1 || h >> levels > 1; ++levels);
	return levels;
}

bool Texture2D::allocate_tex_storage(int w, int h, gli::format gli_format, int levels) {
	glGenTextures(1, &id);
	if (id == 0) {
		ste_log_error() << "Creating texture failed!";
		assert(false);
		return false;
	}

	gli::gl::format const format = GL.translate(gli_format);

	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels - 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, format.Swizzle[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, format.Swizzle[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, format.Swizzle[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, format.Swizzle[3]);
	glTexStorage2D(GL_TEXTURE_2D, levels, format.Internal, w, h);

	this->w = w;
	this->h = h;
	this->format = gli_format;
	this->levels = levels;

	return true;
}

bool Texture2D::upload(const gli::texture2D &texture, bool gm) {
	gli::gl::format const format = GL.translate(texture.format());

	// Index of max level
	int levels = static_cast<GLint>(texture.levels() - 1);
	if (gm) {
		int w = texture.dimensions().x;
		int h = texture.dimensions().y;
		levels = calculate_mipmap_max_level(w, h);
	}

	if (texture.format() != this->format || texture.dimensions().x != this->w || texture.dimensions().y != this->h || levels+1 > this->levels) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	glBindTexture(GL_TEXTURE_2D, id);
	if (gli::is_compressed(texture.format())) {
		for (std::size_t l = 0; l < (generate_mipmaps ? 1 : texture.levels()); ++l) {
			glCompressedTexSubImage2D(GL_TEXTURE_2D, static_cast<GLint>(l),
									  0, 0,
									  static_cast<GLsizei>(texture[l].dimensions().x),
									  static_cast<GLsizei>(texture[l].dimensions().y),
									  format.External,
									  static_cast<GLsizei>(texture[l].size()),
									  texture[l].data());
		}
	}
	else {
		for (std::size_t l = 0; l < (generate_mipmaps ? 1 : texture.levels()); ++l) {
			glTexSubImage2D(GL_TEXTURE_2D, static_cast<GLint>(l),
							0, 0,
							static_cast<GLsizei>(texture[l].dimensions().x),
							static_cast<GLsizei>(texture[l].dimensions().y),
							format.External, format.Type,
							texture[l].data());
		}
	}

	// Generate mipmaps
	if (gm)
		generate_mipmaps();

	return true;
}
