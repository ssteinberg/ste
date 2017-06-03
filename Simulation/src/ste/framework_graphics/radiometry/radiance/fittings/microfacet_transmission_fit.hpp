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

	virtual gli::texture2d_array create_lut() const = 0;

	auto ndf_type() const { return fit_data->header.ndf_type; }
};

struct microfacet_transmission_fit_v4_element {
	double a, b, c, d, m;
};

class microfacet_transmission_fit_v4 : public microfacet_transmission_fit<4, microfacet_transmission_fit_v4_element> {
	using Base = microfacet_transmission_fit<4, microfacet_transmission_fit_v4_element>;

public:
	using Base::Base;

	gli::texture2d_array create_lut() const override {
		using f4_coef = struct { float a, b, c, d; };

		int size = fit_data->header.size;

		gli::texture2d_array lut = gli::texture2d_array(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, glm::ivec2{ size, size }, 2);

		auto *lut0 = reinterpret_cast<f4_coef*>(lut[0].data());
		auto *lut1 = reinterpret_cast<f4_coef*>(lut[1].data());

		for (int y = 0; y < size; ++y) {
			for (int x = 0; x < size; ++x) {
				auto offset = x + y * size;

				lut0[offset] = { static_cast<float>(fit_data->data[offset].a),
								 static_cast<float>(fit_data->data[offset].b),
								 static_cast<float>(fit_data->data[offset].c),
								 .0f };
				lut1[offset] = { static_cast<float>(fit_data->data[offset].d),
								 static_cast<float>(fit_data->data[offset].m),
								.0f ,
								.0f };
			}
		}

		return lut;
	}
};

}
}
