// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <glyph.hpp>
#include <font.hpp>

#include <lib/unique_ptr.hpp>
#include <lib/string.hpp>

namespace ste {
namespace text {

struct glyph_factory_impl;

class glyph_factory {
private:
	lib::unique_ptr<glyph_factory_impl> pimpl;

public:
	glyph_factory();
	~glyph_factory() noexcept;

	glyph_factory(glyph_factory&&) = default;
	glyph_factory &operator=(glyph_factory&&) = default;

	glyph create_glyph(const font &font, wchar_t codepoint) const;

	std::uint32_t read_kerning(const font &font, const std::pair<wchar_t, wchar_t> &p, std::uint32_t pixel_size);
};

}
}
