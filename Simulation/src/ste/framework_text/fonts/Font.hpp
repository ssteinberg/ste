// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <string>

#include "boost_filesystem.hpp"

namespace StE {
namespace Text {

class Font {
private:
	boost::filesystem::path path;
	std::string name;

public:
	Font(const boost::filesystem::path &path);

	bool operator==(const Font& lhs) const { return name == lhs.name; }

	const boost::filesystem::path &get_path() const { return path; }
	const std::string &get_name() const { return name; }
};

}
}

namespace std {

template<>
struct hash<StE::Text::Font> {
	using K = StE::Text::Font;
	std::size_t operator()(const K& k) const {
		return hash<std::string>()(k.get_name());
	}
};

}
