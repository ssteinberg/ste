// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "microfacet_fit_error.hpp"

#include <memory>
#include <istream>

#include <boost/crc.hpp>

namespace StE {
namespace Graphics {

class microfacet_transmission_fit {
	struct transmission_fit_data {
		struct transmission_fit_gauss2 {
			double a1, b1, c1;
			double a2, b2, c2;
		};

		using data_ptr = std::unique_ptr<transmission_fit_gauss2[]>;

		struct {
			std::uint8_t ndf_type[8];
			std::uint16_t version;
			std::uint16_t size;
			std::uint32_t hash;
		} header;
		data_ptr data;

		void alloc_with_size(std::size_t size) {
			data = data_ptr(new transmission_fit_gauss2[size * size]);
		}
	};

private:
	std::unique_ptr<transmission_fit_data> fit_data;

public:
	microfacet_transmission_fit(std::istream &is) {
		fit_data = std::make_unique<transmission_fit_data>();

		is.read(reinterpret_cast<char*>(&fit_data->header), sizeof(fit_data->header) / sizeof(char));

		if (fit_data->header.version != 3)
			throw microfacet_fit_error("Unsupported version");
		if (fit_data->header.size == 0)
			throw microfacet_fit_error("LUT size zero");

		std::size_t lut_row_size = fit_data->header.size;
		fit_data->alloc_with_size(lut_row_size);

		auto lut_size = lut_row_size * lut_row_size * sizeof(transmission_fit_data::transmission_fit_gauss2);
		is.read(reinterpret_cast<char*>(fit_data->data.get()), lut_size / sizeof(char));

		boost::crc_32_type crc_computer;
		crc_computer.process_bytes(reinterpret_cast<const std::uint8_t*>(fit_data->data.get()), lut_size);
		auto hash = crc_computer.checksum();

		if (fit_data->header.hash != hash)
			throw microfacet_fit_error("Checksum mismatch");
	}

	microfacet_transmission_fit(microfacet_transmission_fit &&) = default;
	microfacet_transmission_fit& operator=(microfacet_transmission_fit &&) = default;

	~microfacet_transmission_fit() noexcept {}

	gli::texture2d_array create_lut() const {
		using gauss_coef_f = struct { float a, b, c; };

		int size = fit_data->header.size;

		gli::texture2d_array lut = gli::texture2d_array(gli::format::FORMAT_RGB32_SFLOAT_PACK32, glm::ivec2{ size, size }, 2);

		auto *lut0 = reinterpret_cast<gauss_coef_f*>(lut[0].data());
		auto *lut1 = reinterpret_cast<gauss_coef_f*>(lut[1].data());

		for (int y = 0; y < size; ++y) {
			for (int x = 0; x < size; ++x) {
				auto offset = x + y * size;

				lut0[offset] = { static_cast<float>(fit_data->data[offset].a1), 
								 static_cast<float>(fit_data->data[offset].a2),
								 -static_cast<float>(fit_data->data[offset].b1) };
				lut1[offset] = { 1.f / static_cast<float>(fit_data->data[offset].c1),
								 1.f / static_cast<float>(fit_data->data[offset].c2),
								 -static_cast<float>(fit_data->data[offset].b2) };
			}
		}

		return lut;
	}

	auto ndf_type() const { return fit_data->header.ndf_type; }
};

}
}
