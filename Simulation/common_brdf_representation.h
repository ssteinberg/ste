// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/split_member.hpp>

#include <gli/gli.hpp>

namespace StE {
namespace Graphics {

class common_brdf_representation {
private:
	int in_min, in_max;
	gli::texture3D tex;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const {
		ar << in_min;
		ar << in_max;

		ar << tex.dimensions().x;
		ar << tex.dimensions().y;
		ar << tex.dimensions().z;

		auto size = tex.dimensions().x * tex.dimensions().y * tex.dimensions().z * sizeof(float);
		std::string data;
		data.resize(size);
		memcpy(&data[0], tex.data(), size);
		ar << data;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version) {
		ar >> in_min;
		ar >> in_max;

		unsigned x, y, z;
		ar >> x;
		ar >> y;
		ar >> z;
		tex = gli::texture3D(1, gli::format::FORMAT_R32_SFLOAT, { x,y,z });
		auto size = x*y*z*sizeof(float);

		std::string data;
		ar >> data;
		if (data.size() != size)
			throw new std::exception("Failed to deserialize brdf_data");
		memcpy(tex.data(), &data[0], size);
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
