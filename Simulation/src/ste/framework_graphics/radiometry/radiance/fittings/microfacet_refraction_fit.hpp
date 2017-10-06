//  StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <microfacet_fit_error.hpp>

#include <surface.hpp>

#include <lib/unique_ptr.hpp>
#include <istream>
#include <crc32.h>

namespace ste {
namespace graphics {

class microfacet_refraction_fit {
	struct refraction_fit_data {
		struct refraction_fit_exp2 {
			double a, b, c, d;
		};

		using data_ptr = lib::unique_ptr<refraction_fit_exp2[]>;

		struct {
			std::uint8_t ndf_type[8];
			std::uint16_t version;
			std::uint16_t size;
			std::uint32_t hash;
		} header;
		data_ptr data;

		void alloc_with_size(std::size_t size) {
			data = lib::allocate_unique<refraction_fit_exp2[]>(size * size);
		}
	};

private:
	lib::unique_ptr<refraction_fit_data> fit_data;

public:
	microfacet_refraction_fit(std::istream &is) {
		fit_data = lib::allocate_unique<refraction_fit_data>();

		is.read(reinterpret_cast<char*>(&fit_data->header), sizeof(fit_data->header) / sizeof(char));

		if (fit_data->header.version != 3)
			throw microfacet_fit_error("Unsupported version");
		if (fit_data->header.size == 0)
			throw microfacet_fit_error("LUT size zero");

		const std::size_t lut_row_size = fit_data->header.size;
		fit_data->alloc_with_size(lut_row_size);

		const auto lut_size = lut_row_size * lut_row_size * sizeof(refraction_fit_data::refraction_fit_exp2);
		const auto bytes_read = is.read(reinterpret_cast<char*>(fit_data->data.get()), lut_size / sizeof(char)).gcount();
		if (bytes_read != lut_size)
			throw microfacet_fit_error("Premature EOF");

		// CRC checksum
		const auto hash = crc32::crc32_fast(fit_data->data.get(), lut_size);
		if (fit_data->header.hash != hash)
			throw microfacet_fit_error("Checksum mismatch");
	}

	microfacet_refraction_fit(microfacet_refraction_fit &&) = default;
	microfacet_refraction_fit& operator=(microfacet_refraction_fit &&) = default;

	~microfacet_refraction_fit() noexcept {}

	auto create_lut() const {
		using exp2_coef_f = struct { float a, b, c, d; };

		const auto size = static_cast<std::uint32_t>(fit_data->header.size);

		auto lut_texture = resource::surface_2d<gl::format::r32g32b32a32_sfloat>({ size, size });
		auto lut0 = lut_texture[0_mip];

		for (std::uint32_t y = 0; y < size; ++y) {
			for (std::uint32_t x = 0; x < size; ++x) {
				const auto offset = x + y * size;

				lut0.at({ x,y }).r() = static_cast<float>(fit_data->data[offset].a);
				lut0.at({ x,y }).g() = static_cast<float>(fit_data->data[offset].b);
				lut0.at({ x,y }).b() = static_cast<float>(fit_data->data[offset].c);
				lut0.at({ x,y }).a() = static_cast<float>(fit_data->data[offset].d);
			}
		}

		return lut_texture;
	}

	auto ndf_type() const { return fit_data->header.ndf_type; }
};

}
}
