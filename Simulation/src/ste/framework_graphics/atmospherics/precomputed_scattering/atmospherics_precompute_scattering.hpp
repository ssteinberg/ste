//  StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <atmospherics_lut_error.hpp>
#include <atmospherics_properties.hpp>

#include <surface.hpp>

#include <filesystem>

#include <lib/unique_ptr.hpp>
#include <fstream>
#include <crc32.h>

namespace ste {
namespace graphics {

namespace _detail {

template<typename T>
class _atmospherics_precompute_scattering_data {
private:
	static constexpr int expected_version = 7;	// Current known LUT version

private:
	struct double_vec3 {
		glm::tvec3<T> v0, v1;
	};

public:
	static constexpr std::uint32_t optical_length_size = 2048;
	static constexpr std::uint32_t scatter_size0 = 48;
	static constexpr std::uint32_t scatter_size1 = 384;
	static constexpr std::uint32_t scatter_size2 = 384;
	static constexpr std::uint32_t ambient_size_0 = 48;
	static constexpr std::uint32_t ambient_size_1 = 384;
	static constexpr std::uint32_t ambient_size_2 = 48;

	using optical_length_element = T;
	using scatter_element = double_vec3;
	using ambient_element = glm::tvec3<T>;

	using optical_length_lut_t = optical_length_element[optical_length_size][optical_length_size];
	using scatter_lut_t = scatter_element[scatter_size0][scatter_size1][scatter_size2];
	using ambient_lut_t = ambient_element[ambient_size_0][ambient_size_1][ambient_size_2];

private:
	unsigned char type[8];
	std::uint16_t version;
	std::uint8_t  sizeof_scalar;
	std::uint32_t optical_length_dims;
	std::uint32_t scatter_dims_0;
	std::uint32_t scatter_dims_1;
	std::uint32_t scatter_dims_2;
	std::uint32_t hash;
	std::uint32_t scatter_index;

	atmospherics_properties<T> ap;

	struct {
		optical_length_lut_t optical_length[2];
		scatter_lut_t scatter;
		ambient_lut_t ambient;
	} data;

public:
	_atmospherics_precompute_scattering_data() = default;
	~_atmospherics_precompute_scattering_data() noexcept {}

	static T height_to_lut_idx(const T &h, const T &h_max) {
		auto t = glm::clamp(h, static_cast<T>(0), h_max);
		auto x = t / h_max;
		return glm::sqrt(x);
	}
	static T view_zenith_to_lut_idx(const T &cos_phi) {
		return (static_cast<T>(1) + cos_phi) / static_cast<T>(2);
	}
	static T sun_zenith_to_lut_idx(const T &cos_delta) {
		auto t = -static_cast<T>(2.8) * cos_delta - static_cast<T>(0.8);
		return (static_cast<T>(1) - glm::exp(t)) / (static_cast<T>(1) - glm::exp(static_cast<T>(-3.6)));
	}
	static T sun_view_azimuth_to_lut_idx(const T &omega) {
		return omega / glm::pi<T>();
	}

	static T height_for_lut_idx(const T &x, const T &h_max) {
		return h_max*x*x;
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
	const auto *scatter_lut() const { return &data.scatter; }
	const auto *ambient_lut() const { return &data.ambient; }

	void load(const std::experimental::filesystem::path &path) {
		std::ifstream ifs;
		ifs.exceptions(ifs.exceptions() | std::ios::failbit);
		ifs.open(path.string(), std::ios::binary | std::ios::in);

		if (!ifs)
			throw atmospherics_lut_error("Failed opening LUT");

		if (!ifs.read(reinterpret_cast<char*>(this), sizeof(*this) / sizeof(char)))
			throw atmospherics_lut_error("Can not read LUT");
		ifs.close();

		if (this->sizeof_scalar != sizeof(T))
			throw atmospherics_lut_error("Invalid scalar size");

		if (this->version != expected_version)
			throw atmospherics_lut_error("Unsupported version");

		if (this->optical_length_dims != optical_length_size ||
			this->scatter_dims_0 != scatter_size0 ||
			this->scatter_dims_1 != scatter_size1 ||
			this->scatter_dims_2 != scatter_size2)
			throw atmospherics_lut_error("LUT size zero");

		// CRC checksum
		const auto checksum = crc32::crc32_fast(reinterpret_cast<const std::uint8_t*>(&data), sizeof(data));
		if (this->hash != checksum)
			throw atmospherics_lut_error("Hash mismatch");
	}
};

}

class atmospherics_precompute_scattering {
	using T = double;
	using lut_data_t = _detail::_atmospherics_precompute_scattering_data<T>;

	static constexpr int optical_length_air_lut_idx = 0;
	static constexpr int optical_length_aerosols_lut_idx = 1;

private:
	lib::unique_ptr<lut_data_t> data;

private:
	void create_optical_length_lut(int lut_idx, float *lut) const {
		for (int y = 0; y < lut_data_t::optical_length_size; ++y) {
			for (int x = 0; x < lut_data_t::optical_length_size; ++x) {
				*lut = static_cast<float>((*data->optical_length_lut(lut_idx))[x][y]);
				++lut;
			}
		}
	}

public:
	atmospherics_precompute_scattering(const std::experimental::filesystem::path &path) : data(lib::allocate_unique<lut_data_t>()) {
		data->load(path);
	}
	~atmospherics_precompute_scattering() noexcept {}

	auto create_optical_length_lut() const {
		resource::surface_2d_array<gl::format::r32_sfloat> lut_texture_array(glm::u32vec2{ lut_data_t::optical_length_size, lut_data_t::optical_length_size }, 2_layers);

		auto* lut = reinterpret_cast<float*>(lut_texture_array.data());

		create_optical_length_lut(optical_length_air_lut_idx, 
								  lut);
		create_optical_length_lut(optical_length_aerosols_lut_idx, 
								  lut + lut_texture_array.offset_blocks(1_layer, 0_mip));

		return lut_texture_array;
	}

	auto create_scatter_lut() const {
		resource::surface_3d<gl::format::r32g32b32a32_sfloat> lut_texture(glm::u32vec3{ lut_data_t::scatter_size0, lut_data_t::scatter_size1, lut_data_t::scatter_size2 });

		auto *lut = reinterpret_cast<glm::vec4*>(lut_texture.data());

		for (int z = 0; z < lut_data_t::scatter_size2; ++z)
			for (int y = 0; y < lut_data_t::scatter_size1; ++y)
				for (int x = 0; x < lut_data_t::scatter_size0; ++x) {
					auto &v = (*data->scatter_lut())[x][y][z].v0;

					*lut = { static_cast<float>(v.x),
						static_cast<float>(v.y),
						static_cast<float>(v.z),
						.0f
					};
					++lut;
				}

		return lut_texture;
	}

	auto create_mie0_scatter_lut() const {
		resource::surface_3d<gl::format::r32g32b32a32_sfloat> lut_texture(glm::u32vec3{ lut_data_t::scatter_size0, lut_data_t::scatter_size1, lut_data_t::scatter_size2 });

		auto *lut = reinterpret_cast<glm::vec4*>(lut_texture.data());

		for (int z = 0; z < lut_data_t::scatter_size2; ++z)
			for (int y = 0; y < lut_data_t::scatter_size1; ++y)
				for (int x = 0; x < lut_data_t::scatter_size0; ++x) {
					auto &v = (*data->scatter_lut())[x][y][z].v1;

					*lut = { static_cast<float>(v.x),
						static_cast<float>(v.y),
						static_cast<float>(v.z),
						.0f
					};
					++lut;
				}

		return lut_texture;
	}

	auto create_ambient_lut() const {
		resource::surface_3d<gl::format::r32g32b32a32_sfloat> lut_texture(glm::u32vec3{ lut_data_t::ambient_size_0, lut_data_t::ambient_size_1, lut_data_t::ambient_size_2 });

		auto *lut = reinterpret_cast<glm::vec4*>(lut_texture.data());

		for (int z = 0; z < lut_data_t::ambient_size_2; ++z)
			for (int y = 0; y < lut_data_t::ambient_size_1; ++y)
				for (int x = 0; x < lut_data_t::ambient_size_0; ++x) {
					auto &v = (*data->ambient_lut())[x][y][z];

					*lut = { static_cast<float>(v.x),
						static_cast<float>(v.y),
						static_cast<float>(v.z),
						.0f
					};
					++lut;
				}

		return lut_texture;
	}
};

}
}
