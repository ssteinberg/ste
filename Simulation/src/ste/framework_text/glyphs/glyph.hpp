//  StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface.hpp>
#include <surface_copy.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/split_member.hpp>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace text {

class glyph {
private:
	friend class glyph_factory;
	friend class glyph_manager;

public:
	using glyph_distance_field_surface_t = resource::surface_2d<gl::format::r32_sfloat>;

	struct glyph_metrics {
		std::uint32_t width;
		std::uint32_t height;
		std::int32_t start_y;
		std::int32_t start_x;

	private:
		friend class boost::serialization::access;

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar & width;
			ar & height;
			ar & start_y;
			ar & start_x;
		}
	};

	static constexpr int padding = 16;
	static constexpr int ttf_pixel_size = 64;

private:
	glyph_metrics metrics;
	lib::unique_ptr<glyph_distance_field_surface_t> glyph_distance_field{ nullptr };

private:
	friend class boost::serialization::access;

	template <class Archive>
	void save(Archive& ar, const unsigned int version) const {
		assert(glyph_distance_field != nullptr);

		ar << metrics;

		ar << static_cast<std::uint32_t>(glyph_distance_field->extent().x);
		ar << static_cast<std::uint32_t>(glyph_distance_field->extent().y);
		ar << static_cast<std::uint32_t>(glyph_distance_field->levels());

		const auto size = glyph_distance_field->bytes();
		lib::string data;
		data.resize(size);
		std::memcpy(&data[0], (*glyph_distance_field)[0].data(), size);

		ar << data;
	}

	template <class Archive>
	void load(Archive& ar, const unsigned int version) {
		ar >> metrics;

		std::uint32_t w, h;
		std::uint32_t l;
		ar >> w;
		ar >> h;
		ar >> l;
		glyph_distance_field = lib::allocate_unique<glyph_distance_field_surface_t>(glm::u32vec2{ w, h }, l);

		lib::string data;
		ar >> data;
		const auto size = glm::min(data.size(), glyph_distance_field->bytes());

		assert(size == data.size());

		std::memcpy((*glyph_distance_field)[0].data(), &data[0], size);
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();

public:
	glyph() = default;

	glyph(glyph&&) = default;
	glyph& operator=(glyph&&) = default;

	glyph(const glyph& g) : metrics(g.metrics),
	                        glyph_distance_field(lib::allocate_unique<glyph_distance_field_surface_t>(resource::surface_copy::copy_2d(*g.glyph_distance_field))) {}

	glyph& operator=(const glyph& g) {
		metrics = g.metrics;
		glyph_distance_field = lib::allocate_unique<glyph_distance_field_surface_t>(resource::surface_copy::copy_2d(*g.glyph_distance_field));
		return *this;
	}

	bool empty() const { return glyph_distance_field == nullptr; }
};

}
}
