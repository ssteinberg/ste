// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <string>

#define BOOST_FILESYSTEM_NO_DEPRECATED 
#include <boost/filesystem.hpp>

namespace StE {
namespace Text {

class Font {
private:
	boost::filesystem::path path;

public:
	Font() = default;
	Font(boost::filesystem::path path) : path(path) {}

	const boost::filesystem::path &get_path() const { return path; }

	bool operator==(const Font& lhs) const { return path == lhs.get_path(); }
};

}
}

namespace std {

template<>
struct hash<StE::Text::Font> {
	using K = StE::Text::Font;
	std::size_t operator()(const K& k) const {
		return hash<std::string>()(k.get_path().string());
	}
};

}
