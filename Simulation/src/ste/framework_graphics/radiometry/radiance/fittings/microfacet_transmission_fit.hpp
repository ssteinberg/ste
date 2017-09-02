// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <microfacet_fit_error.hpp>

#include <lib/unique_ptr.hpp>
#include <istream>

#include <boost/crc.hpp>

namespace ste {
namespace graphics {

template <int Version, typename DataType>
class microfacet_transmission_fit {
	struct transmission_fit_data {
		using data_ptr = lib::unique_ptr<DataType[]>;

		struct {
			std::uint8_t ndf_type[8];
			std::uint16_t version;
			std::uint16_t size;
			std::uint32_t hash;
		} header;
		data_ptr data;

		void alloc_with_size(std::size_t size) {
			data = data_ptr(new DataType[size * size]);
		}
	};

protected:
	lib::unique_ptr<transmission_fit_data> fit_data;

public:
	microfacet_transmission_fit(std::istream &is) {
		fit_data = lib::allocate_unique<transmission_fit_data>();

		is.read(reinterpret_cast<char*>(&fit_data->header), sizeof(fit_data->header) / sizeof(char));

		if (fit_data->header.version != Version)
			throw microfacet_fit_error("Unsupported version");
		if (fit_data->header.size == 0)
			throw microfacet_fit_error("LUT size zero");

		std::size_t lut_row_size = fit_data->header.size;
		fit_data->alloc_with_size(lut_row_size);

		auto lut_size = lut_row_size * lut_row_size * sizeof(DataType);
		auto bytes_read = is.read(reinterpret_cast<char*>(fit_data->data.get()), lut_size / sizeof(char)).gcount();
		if (bytes_read != lut_size)
			throw microfacet_fit_error("Premature EOF");

		boost::crc_32_type crc_computer;
		crc_computer.process_bytes(reinterpret_cast<const std::uint8_t*>(fit_data->data.get()), lut_size);
		auto hash = crc_computer.checksum();

		if (fit_data->header.hash != hash)
			throw microfacet_fit_error("Checksum mismatch");
	}

	microfacet_transmission_fit(microfacet_transmission_fit &&) = default;
	microfacet_transmission_fit& operator=(microfacet_transmission_fit &&) = default;

	virtual ~microfacet_transmission_fit() noexcept {}

	virtual resource::surface_2d_array<gl::format::r32g32b32a32_sfloat> create_lut() const = 0;

	auto ndf_type() const { return fit_data->header.ndf_type; }
};

struct microfacet_transmission_fit_v4_element {
	double a, b, c, d, m;
};

class microfacet_transmission_fit_v4 : public microfacet_transmission_fit<4, microfacet_transmission_fit_v4_element> {
	using Base = microfacet_transmission_fit<4, microfacet_transmission_fit_v4_element>;

public:
	using Base::Base;

	resource::surface_2d_array<gl::format::r32g32b32a32_sfloat> create_lut() const override {
		const auto size = static_cast<std::uint32_t>(fit_data->header.size);

		auto lut = resource::surface_2d_array<gl::format::r32g32b32a32_sfloat>({ size, size }, 2);
		auto lut0 = lut[0][0];
		auto lut1 = lut[1][0];

		for (std::uint32_t y = 0; y < size; ++y) {
			for (std::uint32_t x = 0; x < size; ++x) {
				const auto offset = x + y * size;

				lut0.at({ x,y }).r() = static_cast<float>(fit_data->data[offset].a);
				lut0.at({ x,y }).g() = static_cast<float>(fit_data->data[offset].b);
				lut0.at({ x,y }).b() = static_cast<float>(fit_data->data[offset].c);
				lut0.at({ x,y }).a() = .0f;

				lut1.at({ x,y }).r() = static_cast<float>(fit_data->data[offset].d);
				lut1.at({ x,y }).g() = static_cast<float>(fit_data->data[offset].m);
				lut1.at({ x,y }).b() = .0f;
				lut1.at({ x,y }).a() = .0f;
			}
		}

		return lut;
	}
};

}
}
