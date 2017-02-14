// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <boost_binary_ioarchive.hpp>

#include <memory>
#include <string>

#include <chrono>

namespace StE {
namespace Resource {
namespace glsl_loader {

class program_binary {
public:
	GLenum format;
	std::uint64_t time;
	std::string blob;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & format;
		ar & time;
		ar & blob;
	}

public:
	std::chrono::system_clock::time_point get_time_point() const {
		return std::chrono::system_clock::time_point(std::chrono::seconds(time));
	}

	void set_time_point(const std::chrono::system_clock::time_point &tp) {
		time = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
	}
};

}
}
}
