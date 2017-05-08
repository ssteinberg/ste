// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <string>

#include <boost_filesystem.hpp>

namespace ste {
namespace text {

class font {
private:
	boost::filesystem::path path;
	std::string name;

public:
	font(const boost::filesystem::path &path);

	bool operator==(const font& lhs) const { return name == lhs.name; }

	const boost::filesystem::path &get_path() const { return path; }
	const std::string &get_name() const { return name; }
};

}
}

namespace std {

template<>
struct hash<ste::text::font> {
	using K = ste::text::font;
	std::size_t operator()(const K& k) const {
		return hash<std::string>()(k.get_name());
	}
};

}
