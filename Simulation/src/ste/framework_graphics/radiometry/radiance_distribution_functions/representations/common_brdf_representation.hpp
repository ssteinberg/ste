// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/split_member.hpp>

#include <stdexcept>
#include <memory>

namespace StE {
namespace Graphics {

class common_brdf_representation {
private:
	int in_min, in_max;
	std::unique_ptr<gli::texture3d> tex;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const {
		ar << in_min;
		ar << in_max;

		auto dim = tex->extent();
		ar << static_cast<int>(dim.x);
		ar << static_cast<int>(dim.y);
		ar << static_cast<int>(dim.z);

		auto size = dim.x * dim.y * dim.z * sizeof(float);
		std::string data;
		data.resize(size);
		memcpy(&data[0], tex->data(), size);
		ar << data;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version) {
		ar >> in_min;
		ar >> in_max;

		int x, y, z;
		ar >> x;
		ar >> y;
		ar >> z;
		tex = std::make_unique<gli::texture3d>(gli::format::FORMAT_R32_SFLOAT_PACK32, glm::ivec3{ x,y,z }, 1);
		auto size = x*y*z*sizeof(float);

		std::string data;
		ar >> data;
		if (data.size() != size)
			throw new std::runtime_error("Failed to deserialize brdf_data");
		memcpy(tex->data(), &data[0], size);
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER();

public:
	void set_min_incident(int angle) { in_min = angle; }
	void set_max_incident(int angle) { in_max = angle; }

	int get_min_incident() const { return in_min; }
	int get_max_incident() const { return in_max; }
	auto &get_data() { return tex; }
	const auto &get_data() const { return tex; }
};

}
}
