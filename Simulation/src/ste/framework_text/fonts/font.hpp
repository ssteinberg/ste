// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <lib/string.hpp>

#include <filesystem>

namespace ste {
namespace text {

class font {
private:
	std::experimental::filesystem::path path;
	lib::string name;

public:
	font(const std::experimental::filesystem::path &path);

	bool operator==(const font& lhs) const { return name == lhs.name; }
	bool operator!=(const font& lhs) const { return name != lhs.name; }

	const std::experimental::filesystem::path &get_path() const { return path; }
	const lib::string &get_name() const { return name; }
};

}
}

namespace std {

template<>
struct hash<ste::text::font> {
	using K = ste::text::font;
	std::size_t operator()(const K& k) const {
		return hash<ste::lib::string>()(k.get_name());
	}
};

}
