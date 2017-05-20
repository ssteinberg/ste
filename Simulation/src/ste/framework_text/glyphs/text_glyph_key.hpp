// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <text_font_key.hpp>

namespace ste {
namespace text {

namespace _internal {

struct glyph_key {
	lib::string fname;
	wchar_t codepoint;

	glyph_key() = default;
	glyph_key(const font_key &f, wchar_t codepoint) : fname(f.fname), codepoint(codepoint) {}
	bool operator<(const glyph_key &rhs) const {
		if (codepoint == rhs.codepoint)
			return fname < rhs.fname;
		return codepoint < rhs.codepoint;
	}

	friend inline bool operator==(const glyph_key &lhs, const glyph_key &rhs) { return lhs.fname == rhs.fname && lhs.codepoint == rhs.codepoint; }
	friend inline bool operator!=(const glyph_key &lhs, const glyph_key &rhs) { return !(lhs == rhs); }
};

}

}
}
