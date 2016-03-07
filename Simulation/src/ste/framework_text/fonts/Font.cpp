
#include "stdafx.h"
#include "Font.h"

#include <iostream>
#include <stdexcept>

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace StE::Text;

Font::Font(const boost::filesystem::path &path) : path(path) {
	FT_Library library;
	auto error = FT_Init_FreeType(&library);
	if (error) {
		throw std::runtime_error("Couldn't init libfreetype");
	}

	FT_Face face;
	error = FT_New_Face(library,
						path.string().c_str(),
						0,
						&face);
	if (error) {
		throw std::runtime_error("Failed loading font");
	}

	if (face->family_name && face->style_name)
		name = std::string(face->family_name) + " " + face->style_name;
	else
		name = path.filename().string();

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}
