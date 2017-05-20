// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <text_font_key.hpp>

namespace ste {
namespace text {

namespace _internal {

struct sized_glyph_pair_key {
	lib::string fname;
	wchar_t left;
	wchar_t right;
	std::uint32_t pixel_size;

	sized_glyph_pair_key() = default;
	sized_glyph_pair_key(const font_key &f, wchar_t left, wchar_t right, std::uint32_t pixel_size)
		: fname(f.fname), left(left), right(right), pixel_size(pixel_size)
	{}

	bool operator<(const sized_glyph_pair_key &rhs) const {
		auto tag_lhs = (static_cast<std::uint64_t>(left) << 48) + (static_cast<std::uint64_t>(right) << 32) + static_cast<std::uint64_t>(pixel_size);
		auto tag_rhs = (static_cast<std::uint64_t>(rhs.left) << 48) + (static_cast<std::uint64_t>(rhs.right) << 32) + static_cast<std::uint64_t>(rhs.pixel_size);

		if (tag_lhs == tag_rhs)
			return fname < rhs.fname;
		return tag_lhs < tag_rhs;
	}

	friend inline bool operator==(const sized_glyph_pair_key &lhs, const sized_glyph_pair_key &rhs) {
		return lhs.right == rhs.right &&
			lhs.left == rhs.left &&
			lhs.pixel_size == rhs.pixel_size &&
			lhs.fname == rhs.fname;
	}
	friend inline bool operator!=(const sized_glyph_pair_key &lhs, const sized_glyph_pair_key &rhs) { return !(lhs == rhs); }
};

}

}
}
