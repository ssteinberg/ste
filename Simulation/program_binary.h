// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <memory>
#include <string>

namespace StE {
namespace Resource {
namespace glsl_loader {

struct program_binary {
	GLenum format;
	std::uint64_t crc;
	std::string blob;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & format;
		ar & crc;
		ar & blob;
	}
};

}
}
}
