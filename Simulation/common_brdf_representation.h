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
	gli::texture2DArray tex;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const {
		ar << in_min;
		ar << in_max;

		ar << tex.dimensions().x;
		ar << tex.dimensions().y;
		ar << tex.layers();

		auto size = tex.dimensions().x * tex.dimensions().y * sizeof(float);
		for (unsigned l = 0; l < tex.layers(); ++l) {
			std::string data;
			data.resize(size);
			memcpy(&data[0], tex[l].data(), size);
			ar << data;
		}
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version) {
		ar >> in_min;
		ar >> in_max;

		unsigned x, y, z;
		ar >> x;
		ar >> y;
		ar >> z;
		tex = gli::texture2DArray(z, 1, gli::format::FORMAT_R32_SFLOAT, { x,y });
		auto size = x*y*sizeof(float);

		for (unsigned l = 0; l < z; ++l) {
			std::string data;
			ar >> data;
			if (data.size() != size)
				throw new std::exception("Failed to deserialize brdf_data");
			memcpy(tex[l].data(), &data[0], size);
		}
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
