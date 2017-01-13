// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "atmospherics_lut_error.hpp"

#include "boost_filesystem.hpp"

#include <memory>
#include <fstream>
#include <boost/crc.hpp>

namespace StE {
namespace Graphics {

namespace _detail {

template<typename T>
class _atmospherics_precompute_scattering_data {
public:
	static constexpr int optical_length_size = 1024;
	static constexpr int scatter_size0 = 32;
	static constexpr int scatter_size1 = 128;
	static constexpr int scatter_size2 = 32;
	//static constexpr int scatter_size3 = 32;

	using scatter_element = glm::tvec3<T>;

	using optical_length_lut_t = T[optical_length_size][optical_length_size];
	using scatter_lut_t = scatter_element[scatter_size0][scatter_size1][scatter_size2];// [scatter_size3];
	using mie_single_scatter_lut_t = scatter_element[scatter_size0][scatter_size1][scatter_size2];// [scatter_size3];

private:
	unsigned char type[8];
	std::uint16_t version;
	std::uint8_t  _unused;
	std::uint32_t optical_length_dims;
	std::uint32_t scatter_dims_0;
	std::uint32_t scatter_dims_1;
	std::uint32_t scatter_dims_2;
	std::uint32_t hash;

	struct {
		optical_length_lut_t optical_length[2];
		scatter_lut_t multi_scatter;
		mie_single_scatter_lut_t mie_single_scatter;
	} data;

public:
	_atmospherics_precompute_scattering_data() {}
	~_atmospherics_precompute_scattering_data() noexcept {}

	static T height_to_lut_idx(const T &h, const T &h_max) {
		auto t = glm::clamp(h, static_cast<T>(0), h_max);
		return t / h_max;
	}
	static T view_zenith_to_lut_idx(const T &phi) {
		return (static_cast<T>(1) + glm::cos(phi)) / static_cast<T>(2);
	}
	static T sun_zenith_to_lut_idx(const T &delta) {
		auto t = -static_cast<T>(2.8) * glm::cos(delta) - static_cast<T>(0.8);
		return (static_cast<T>(1) - glm::exp(t)) / (static_cast<T>(1) - glm::exp(static_cast<T>(-3.6)));
	}
	static T sun_view_azimuth_to_lut_idx(const T &omega) {
		return omega / glm::pi<T>();
	}

	static T height_for_lut_idx(const T &x, const T &h_max) {
		return h_max*x;
	}
	static T view_zenith_for_lut_idx(const T &x) {
		return glm::acos(static_cast<T>(2) * x - static_cast<T>(1));
	}
	static T sun_zenith_for_lut_idx(const T &x) {
		if (x == 1) return 0;
		auto t = static_cast<T>(1) - x*(static_cast<T>(1) - glm::exp(static_cast<T>(-3.6)));
		return glm::acos(-(glm::log(t) + static_cast<T>(0.8)) / static_cast<T>(2.8));
	}
	static T sun_view_azimuth_for_lut_idx(const T &x) {
		return static_cast<T>(x) * glm::pi<T>();
	}

	const auto *optical_length_lut(int lut) const { return &(data.optical_length[lut]); }
	const auto *multi_scatter_lut() const { return &data.multi_scatter; }
	const auto *m0_scatter_lut() const { return &data.mie_single_scatter; }

	void load(const boost::filesystem::path &path) {
		std::ifstream ifs;
		ifs.exceptions(ifs.exceptions() | std::ios::failbit);
		ifs.open(path.string(), std::ios::binary | std::ios::in);

		if (!ifs.read(reinterpret_cast<char*>(this), sizeof(*this) / sizeof(char)))
			throw atmospherics_lut_error("Can not read LUT");
		ifs.close();

		boost::crc_32_type crc_computer;
		crc_computer.process_bytes(reinterpret_cast<const std::uint8_t*>(&data), sizeof(data));
		if (this->hash != crc_computer.checksum())
			throw atmospherics_lut_error("Hash mismatch");

		if (this->version != 1)
			throw atmospherics_lut_error("Unsupported version");
		if (this->optical_length_dims != optical_length_size ||
			this->scatter_dims_0 != scatter_size0 ||
			this->scatter_dims_1 != scatter_size1 ||
			this->scatter_dims_2 != scatter_size2)
			throw atmospherics_lut_error("LUT size zero");
	}
};

}

class atmospherics_precompute_scattering {
	using T = double;
	using lut_data_t = _detail::_atmospherics_precompute_scattering_data<T>;

	static constexpr int optical_length_air_lut_idx = 0;
	static constexpr int optical_length_aerosols_lut_idx = 1;

private:
	std::unique_ptr<lut_data_t> data;

private:
	gli::texture2d create_optical_length_lut(int lut_idx) const {
		gli::texture2d lut_texture = gli::texture2d(gli::format::FORMAT_R32_SFLOAT_PACK32,
													glm::ivec2{ lut_data_t::optical_length_size });

		auto *lut = reinterpret_cast<float*>(lut_texture.data());

		for (int y = 0; y < lut_data_t::optical_length_size; ++y) {
			for (int x = 0; x < lut_data_t::optical_length_size; ++x) {
				*lut = (*data->optical_length_lut(lut_idx))[x][y];
				++lut;
			}
		}

		return lut_texture;
	}

public:
	atmospherics_precompute_scattering(const boost::filesystem::path &path) : data(std::make_unique<lut_data_t>()) {
		data->load(path);
	}
	~atmospherics_precompute_scattering() noexcept {}

	gli::texture2d_array create_optical_length_lut() const {
		auto lut_array = gli::texture2d_array(gli::format::FORMAT_R32_SFLOAT_PACK32,
											  glm::ivec2{ lut_data_t::optical_length_size }, 2);
		lut_array[0] = create_optical_length_lut(optical_length_air_lut_idx);
		lut_array[1] = create_optical_length_lut(optical_length_aerosols_lut_idx);

		return lut_array;
	}

	gli::texture3d create_lut() const {
		auto lut_texture = gli::texture3d(gli::format::FORMAT_RGB32_SFLOAT_PACK32,
										  glm::ivec3{ lut_data_t::scatter_size0, lut_data_t::scatter_size1, lut_data_t::scatter_size2 });

		auto *lut = reinterpret_cast<glm::vec3*>(lut_texture.data());

		for (int z = 0; z < lut_data_t::scatter_size2; ++z) {
			for (int y = 0; y < lut_data_t::scatter_size1; ++y) {
				for (int x = 0; x < lut_data_t::scatter_size0; ++x) {
					auto &v = (*data->multi_scatter_lut())[x][y][z];
					*lut = { static_cast<float>(v.x),
						static_cast<float>(v.y),
						static_cast<float>(v.z) };
					++lut;
				}
			}
		}

		return lut_texture;
	}
};

}
}
