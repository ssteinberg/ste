// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <font.hpp>

namespace ste {
namespace text {

namespace _internal {

struct font_key {
	lib::string fname;

	font_key() = default;
	font_key(const font &f) : fname(f.get_name()) {}
	bool operator<(const font_key &rhs) const {
		return fname < rhs.fname;
	}

	friend inline bool operator==(const font_key &lhs, const font_key &rhs) { return lhs.fname == rhs.fname; }
	friend inline bool operator!=(const font_key &lhs, const font_key &rhs) { return lhs.fname != rhs.fname; }

	friend inline bool operator==(const font &f, const font_key &k) { return f.get_name() == k.fname; }
	friend inline bool operator!=(const font &f, const font_key &k) { return f.get_name() != k.fname; }
	friend inline bool operator==(const font_key &k, const font &f) { return k.fname == f.get_name(); }
	friend inline bool operator!=(const font_key &k, const font &f) { return k.fname != f.get_name(); }
};

}

}
}
