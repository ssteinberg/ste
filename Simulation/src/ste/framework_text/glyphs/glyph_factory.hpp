// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <glyph.hpp>
#include <font.hpp>

#include <memory>
#include <string>

namespace ste {
namespace text {

struct glyph_factory_impl;

class glyph_factory {
private:
	std::unique_ptr<glyph_factory_impl> pimpl;

public:
	glyph_factory();
	~glyph_factory();

	glyph create_glyph(const font &font, wchar_t codepoint);

	int read_kerning(const font &font, const std::pair<wchar_t, wchar_t> &p, int pixel_size);
};

}
}
