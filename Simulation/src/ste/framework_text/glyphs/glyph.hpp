// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <boost_binary_ioarchive.hpp>
#include <boost_serialization.hpp>

#include <memory>

namespace StE {
namespace Text {

class glyph {
private:
	friend class glyph_factory;
	friend class glyph_manager;

public:
	struct glyph_metrics {
		std::int32_t width;
		std::int32_t height;
		std::int32_t start_y;
		std::int32_t start_x;

	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version) {
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
	std::unique_ptr<gli::texture2d> glyph_distance_field;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const {
		ar << metrics;

		ar << static_cast<std::size_t>(glyph_distance_field->extent().x);
		ar << static_cast<std::size_t>(glyph_distance_field->extent().y);
		ar << static_cast<std::int64_t>(glyph_distance_field->levels());
		ar << static_cast<std::int64_t>(glyph_distance_field->format());

		std::size_t size = (*glyph_distance_field)[0].extent().x * (*glyph_distance_field)[0].extent().y * 4;
		std::string data;
		data.resize(size);
		memcpy(&data[0], (*glyph_distance_field)[0].data(), size);
		ar << data;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version) {
		ar >> metrics;

		std::size_t w, h;
		std::int64_t l;
		std::int64_t format;
		ar >> w;
		ar >> h;
		ar >> l;
		ar >> format;
		glyph_distance_field = std::make_unique<gli::texture2d>(static_cast<gli::format>(format), glm::ivec2{ w, h }, static_cast<int>(l));

		std::string data;
		ar >> data;
		memcpy((*glyph_distance_field)[0].data(), &data[0], data.size());
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER();

public:
	glyph() {}

	glyph(glyph &&) = default;
	glyph &operator=(glyph &&) = default;

	glyph(const glyph &g) : metrics(g.metrics), glyph_distance_field(std::make_unique<gli::texture2d>(*g.glyph_distance_field)) {}
	glyph &operator=(const glyph &g) {
		metrics = g.metrics;
		glyph_distance_field = std::make_unique<gli::texture2d>(*g.glyph_distance_field);
		return *this;
	}

	bool empty() const { return glyph_distance_field->empty(); }
};

}
}
