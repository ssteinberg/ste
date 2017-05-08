// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <microfacet_fit_error.hpp>

#include <memory>
#include <istream>

#include <boost/crc.hpp>

namespace ste {
namespace graphics {

class microfacet_refraction_fit {
	struct refraction_fit_data {
		struct refraction_fit_exp2 {
			double a, b, c, d;
		};

		using data_ptr = std::unique_ptr<refraction_fit_exp2[]>;

		struct {
			std::uint8_t ndf_type[8];
			std::uint16_t version;
			std::uint16_t size;
			std::uint32_t hash;
		} header;
		data_ptr data;

		void alloc_with_size(std::size_t size) {
			data = data_ptr(new refraction_fit_exp2[size * size]);
		}
	};

private:
	std::unique_ptr<refraction_fit_data> fit_data;

public:
	microfacet_refraction_fit(std::istream &is) {
		fit_data = std::make_unique<refraction_fit_data>();

		is.read(reinterpret_cast<char*>(&fit_data->header), sizeof(fit_data->header) / sizeof(char));

		if (fit_data->header.version != 3)
			throw microfacet_fit_error("Unsupported version");
		if (fit_data->header.size == 0)
			throw microfacet_fit_error("LUT size zero");

		std::size_t lut_row_size = fit_data->header.size;
		fit_data->alloc_with_size(lut_row_size);

		auto lut_size = lut_row_size * lut_row_size * sizeof(refraction_fit_data::refraction_fit_exp2);
		auto bytes_read = is.read(reinterpret_cast<char*>(fit_data->data.get()), lut_size / sizeof(char)).gcount();
		if (bytes_read != lut_size)
			throw microfacet_fit_error("Premature EOF");

		boost::crc_32_type crc_computer;
		crc_computer.process_bytes(reinterpret_cast<const std::uint8_t*>(fit_data->data.get()), lut_size);
		auto hash = crc_computer.checksum();

		if (fit_data->header.hash != hash)
			throw microfacet_fit_error("Checksum mismatch");
	}

	microfacet_refraction_fit(microfacet_refraction_fit &&) = default;
	microfacet_refraction_fit& operator=(microfacet_refraction_fit &&) = default;

	~microfacet_refraction_fit() noexcept {}

	gli::texture2d create_lut() const {
		using exp2_coef_f = struct { float a, b, c, d; };

		int size = fit_data->header.size;

		gli::texture2d lut_texture = gli::texture2d(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, glm::ivec2{ size, size });

		auto *lut = reinterpret_cast<exp2_coef_f*>(lut_texture.data());

		for (int y = 0; y < size; ++y) {
			for (int x = 0; x < size; ++x) {
				auto offset = x + y * size;

				lut[offset] = { static_cast<float>(fit_data->data[offset].a), 
								static_cast<float>(fit_data->data[offset].b),
								static_cast<float>(fit_data->data[offset].c),
								static_cast<float>(fit_data->data[offset].d) };
			}
		}

		return lut_texture;
	}

	auto ndf_type() const { return fit_data->header.ndf_type; }
};

}
}
