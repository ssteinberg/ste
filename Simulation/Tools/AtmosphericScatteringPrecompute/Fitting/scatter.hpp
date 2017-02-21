// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.h"
#include "atmospheric_properties.hpp"
#include "phase.hpp"

#include "romberg_integration.hpp"
#include "gaussian_quadrature_spherical_integration.hpp"

#include <boost/crc.hpp>

#include <limits>
#include <cmath>

#include <future>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>

#include <glm/gtc/constants.hpp>

namespace StE {
namespace Graphics {


template<typename T>
class double_vec3 {
public:
	glm::tvec3<T> v0, v1;

	double_vec3() = default;
	double_vec3(double_vec3 &&) = default;
	double_vec3(const double_vec3 &) = default;
	double_vec3 &operator=(double_vec3 &&) = default;
	double_vec3 &operator=(const double_vec3 &) = default;

	double_vec3(const T &t) : v0(t), v1(t) {}
	double_vec3(const glm::tvec3<T> &t) : v0(t), v1(t) {}
	double_vec3(const glm::tvec3<T> &t0, const glm::tvec3<T> &t1) : v0(t0), v1(t1) {}

	auto& operator+=(const double_vec3<T> &rhs) {
		v0 += rhs.v0;
		v1 += rhs.v1;
		return *this;
	}
	auto& operator-=(const double_vec3<T> &rhs) {
		v0 -= rhs.v0;
		v1 -= rhs.v1;
		return *this;
	}
	auto& operator+=(const glm::tvec3<T> &rhs) {
		v0 += rhs;
		v1 += rhs;
		return *this;
	}
	auto& operator-=(const glm::tvec3<T> &rhs) {
		v0 -= rhs;
		v1 -= rhs;
		return *this;
	}
	auto& operator*=(const double_vec3<T> &rhs) {
		v0 *= rhs.v0;
		v1 *= rhs.v1;
		return *this;
	}
	auto& operator/=(const double_vec3<T> &rhs) {
		v0 /= rhs.v0;
		v1 /= rhs.v1;
		return *this;
	}
	auto& operator*=(const glm::tvec3<T> &rhs) {
		v0 *= rhs;
		v1 *= rhs;
		return *this;
	}
	auto& operator/=(const glm::tvec3<T> &rhs) {
		v0 /= rhs;
		v1 /= rhs;
		return *this;
	}
	auto& operator*=(const T &rhs) {
		v0 *= rhs;
		v1 *= rhs;
		return *this;
	}
	auto& operator/=(const T &rhs) {
		v0 /= rhs;
		v1 /= rhs;
		return *this;
	}
};

template<typename T>
double_vec3<T> operator+(const double_vec3<T> &lhs, const double_vec3<T> &rhs) {
	return{ lhs.v0 + rhs.v0,
		lhs.v1 + rhs.v1 };
}
template<typename T>
double_vec3<T> operator-(const double_vec3<T> &lhs, const double_vec3<T> &rhs) {
	return{ lhs.v0 - rhs.v0,
		lhs.v1 - rhs.v1 };
}
template<typename T>
double_vec3<T> operator+(const double_vec3<T> &lhs, const glm::tvec3<T> &rhs) {
	return{ lhs.v0 + rhs,
		lhs.v1 + rhs };
}
template<typename T>
double_vec3<T> operator-(const double_vec3<T> &lhs, const glm::tvec3<T> &rhs) {
	return{ lhs.v0 - rhs,
		lhs.v1 - rhs };
}
template<typename T>
double_vec3<T> operator*(const double_vec3<T> &lhs, const double_vec3<T> &rhs) {
	return{ lhs.v0 * rhs.v0,
		lhs.v1 * rhs.v1 };
}
template<typename T>
double_vec3<T> operator/(const double_vec3<T> &lhs, const double_vec3<T> &rhs) {
	return{ lhs.v0 / rhs.v0,
		lhs.v1 / rhs.v1 };
}
template<typename T>
double_vec3<T> operator*(const double_vec3<T> &lhs, const glm::tvec3<T> &rhs) {
	return{ lhs.v0 * rhs,
		lhs.v1 * rhs };
}
template<typename T>
double_vec3<T> operator/(const double_vec3<T> &lhs, const glm::tvec3<T> &rhs) {
	return{ lhs.v0 / rhs,
		lhs.v1 / rhs };
}
template<typename T>
double_vec3<T> operator*(const double_vec3<T> &lhs, const T &rhs) {
	return{ lhs.v0 * rhs,
		lhs.v1 * rhs };
}
template<typename T>
double_vec3<T> operator/(const double_vec3<T> &lhs, const T &rhs) {
	return{ lhs.v0 / rhs,
		lhs.v1 / rhs };
}
template<typename T>
double_vec3<T> operator*(const T &lhs, const double_vec3<T> &rhs) {
	return{ lhs * rhs.v0,
		lhs * rhs.v1 };
}
template<typename T>
double_vec3<T> operator/(const T &lhs, const double_vec3<T> &rhs) {
	return{ lhs / rhs.v0,
		lhs / rhs.v2 };
}


namespace _detail {

template<typename T>
class _atmospherics_precompute_scattering_data {
public:
	static constexpr int optical_length_size = 2048;
	static constexpr int scatter_size0 = 48;
	static constexpr int scatter_size1 = 384;
	static constexpr int scatter_size2 = 384;
	static constexpr int ambient_size_0 = 48;
	static constexpr int ambient_size_1 = 384;
	static constexpr int ambient_size_2 = 48;

	using optical_length_element = T;
	using scatter_element = double_vec3<T>;
	using ambient_element = glm::tvec3<T>;

	using optical_length_lut_t = optical_length_element[optical_length_size][optical_length_size];
	using scatter_lut_t = scatter_element[scatter_size0][scatter_size1][scatter_size2];
	using ambient_lut_t = ambient_element[ambient_size_0][ambient_size_1][ambient_size_2];

private:
	// Header
	mutable unsigned char type[8];
	std::uint16_t version{ 7 };
	std::uint8_t  sizeof_scalar{ sizeof(T) };
	std::uint32_t optical_length_dims{ optical_length_size };
	std::uint32_t scatter_dims_0{ scatter_size0 };
	std::uint32_t scatter_dims_1{ scatter_size1 };
	std::uint32_t scatter_dims_2{ scatter_size2 };
	mutable std::uint32_t hash{ 0 };
	std::uint32_t scatter_index;

	// Atmosphere properties
	atmospherics_properties<T> ap;

	// Data
	struct {
		optical_length_lut_t optical_length[2];
		scatter_lut_t scatter;
		ambient_lut_t ambient;
	} data;

public:
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

	static T ambient_NdotL_to_lut_idx(const T &NdotL) {
		return (static_cast<T>(1) + NdotL) / static_cast<T>(2);
	}
	static T ambient_NdotL_for_lut_idx(const T &x) {
		return static_cast<T>(2) * x - static_cast<T>(1);
	}

public:
	_atmospherics_precompute_scattering_data(const atmospherics_properties<T> &ap) : ap(ap) {}

	void set_scatter_index(int k) {
		scatter_index = k;
	}

	void write_scatter_value(const glm::ivec3 &idx, const double_vec3<T> &s) {
		data.scatter[idx.x][idx.y][idx.z] = s;
	}
	void add_scatter_value(const glm::ivec3 &idx, const glm::tvec3<T> &s) {
		data.scatter[idx.x][idx.y][idx.z].v0 += s;
	}
	void write_ambient_value(const glm::ivec3 &idx, const glm::tvec3<T> &s) {
		data.ambient[idx.x][idx.y][idx.z] = s;
	}
	void write_optical_length_value(int lut, const glm::ivec2 &idx, const T &value) {
		data.optical_length[lut][idx.x][idx.y] = value;
	}
	auto sample_optical_length_value(int lut, const glm::dvec2 &idx) const {
		auto coords = idx * static_cast<double>(optical_length_size - 1);
		auto x = glm::floor(coords);
		auto f = glm::fract(coords);

		auto x0 = static_cast<int>(x.x);
		auto y0 = static_cast<int>(x.y);
		auto x1 = glm::min<int>(optical_length_size-1, x0+1);
		auto y1 = glm::min<int>(optical_length_size-1, y0+1);

		auto s00 = data.optical_length[lut][x0][y0];
		auto s10 = data.optical_length[lut][x1][y0];
		auto s01 = data.optical_length[lut][x0][y1];
		auto s11 = data.optical_length[lut][x1][y1];

		auto t0 = glm::mix(s00, s10, f.x);
		auto t1 = glm::mix(s01, s11, f.x);
		return glm::mix(t0, t1, f.y);
	}

	auto sample_scatter_value(const glm::dvec3 &idx, const glm::tvec3<T> &V, const glm::tvec3<T> &L) const {
		auto coords = idx * glm::dvec3{ scatter_size0 - 1, scatter_size1 - 1, scatter_size2 - 1 };
		auto x = glm::floor(coords);
		auto f = glm::fract(coords);

		scatter_element s[2][2][2];
		for (int i1 = 0; i1 < 2; ++i1)
			for (int i2 = 0; i2 < 2; ++i2)
				for (int i3 = 0; i3 < 2; ++i3) {
					s[i1][i2][i3] = data.scatter[glm::min<int>(scatter_size0 - 1, static_cast<int>(x.x) + i1)]
						[glm::min<int>(scatter_size1 - 1, static_cast<int>(x.y) + i2)]
					[glm::min<int>(scatter_size2 - 1, static_cast<int>(x.z) + i3)];
				}

		for (int i1 = 0; i1 < 2; ++i1)
			for (int i2 = 0; i2 < 2; ++i2) {
				s[0][i1][i2] = { glm::mix(s[0][i1][i2].v0, s[1][i1][i2].v0, f.x),
					glm::mix(s[0][i1][i2].v1, s[1][i1][i2].v1, f.x) };
			}
		for (int i1 = 0; i1 < 2; ++i1) {
			s[0][0][i1] = { glm::mix(s[0][0][i1].v0, s[0][1][i1].v0, f.y),
				glm::mix(s[0][0][i1].v1, s[0][1][i1].v1, f.y) };
		}

		double_vec3<T> sample = { glm::mix(s[0][0][0].v0, s[0][0][1].v0, f.z), glm::mix(s[0][0][0].v1, s[0][0][1].v1, f.z) };

		auto r = sample.v0;
		auto m0 = sample.v1;
		auto c = glm::dot(-V, L);

		return r + m0 * cornette_shanks_phase_function(c, this->ap.phase + .075);
	}

	auto &read_scatter_value(const glm::ivec3 &idx) const {
		return data.scatter[idx.x][idx.y][idx.z];
	}

	void load(const std::string &path) {
		std::ifstream ifs(path, std::ios::binary);
		ifs.read(reinterpret_cast<char*>(this), sizeof(*this));
		ifs.close();

		boost::crc_32_type crc_computer;
		crc_computer.process_bytes(reinterpret_cast<const std::uint8_t*>(&data), sizeof(data));
		if (this->hash != crc_computer.checksum())
			std::cout << "Warning! Hash mismatch!" << std::endl;
	}

	void write_out(const std::string &path) const {
		memcpy(this->type, "ATMS SCT", 8);

		boost::crc_32_type crc_computer;
		crc_computer.process_bytes(reinterpret_cast<const std::uint8_t*>(&data), sizeof(data));
		this->hash = crc_computer.checksum();

		std::ofstream ofs(path, std::ios::binary);
		ofs.write(reinterpret_cast<const char*>(this), sizeof(*this));
		ofs.close();
	}
};

}

class atmospherics_precompute_scattering {
	using T = double;
	using lut_t = _detail::_atmospherics_precompute_scattering_data<T>;

	static constexpr int N = 7;
	static constexpr int M = 24;

	struct temp_lut_t {
		glm::tvec3<T> scatter[lut_t::scatter_size0][lut_t::scatter_size1][lut_t::scatter_size2];
	};

private:
	atmospherics_properties<T> ap;
	T Hmax;

	gaussian_quadrature_spherical_integration<M>::quadrature_points quadrature_points;

	std::unique_ptr<lut_t> final_lut;
	std::unique_ptr<temp_lut_t> last_scatter_lut;
	std::unique_ptr<temp_lut_t> temp_lut;

private:
	static T cie_scattering_indicatrix(const T &x, const T &cos_x) {
		return 1. + 10. * (exp(-3. * x) - exp(-3. * glm::pi<T>() / 2.)) + 0.45 * cos_x*cos_x;
	}
	static T cie_scattering_indicatrix_normalizer() {
		return cie_scattering_indicatrix(.0, 1.0);
	}

	auto sample_last_scatter_value(const glm::dvec3 &idx) const {
		auto coords = idx * glm::dvec3{ lut_t::scatter_size0 - 1, lut_t::scatter_size1 - 1, lut_t::scatter_size2 - 1 };
		auto x = glm::floor(coords);
		auto f = glm::fract(coords);

		glm::tvec3<T> s[2][2][2];
		for (int i1 = 0; i1 < 2; ++i1)
			for (int i2 = 0; i2 < 2; ++i2)
				for (int i3 = 0; i3 < 2; ++i3) {
					s[i1][i2][i3] = last_scatter_lut->scatter[glm::min<int>(lut_t::scatter_size0 - 1, static_cast<int>(x.x) + i1)]
						[glm::min<int>(lut_t::scatter_size1 - 1, static_cast<int>(x.y) + i2)]
					[glm::min<int>(lut_t::scatter_size2 - 1, static_cast<int>(x.z) + i3)];
				}

		for (int i1 = 0; i1 < 2; ++i1)
			for (int i2 = 0; i2 < 2; ++i2) {
				s[0][i1][i2] = glm::mix(s[0][i1][i2], s[1][i1][i2], f.x);
			}
		for (int i1 = 0; i1 < 2; ++i1) {
			s[0][0][i1] = glm::mix(s[0][0][i1], s[0][1][i1], f.y);
		}

		return glm::mix(s[0][0][0], s[0][0][1], f.z);
	}

	static T intersect_line_sphere(const glm::tvec3<T> &c, const T &r,
								   const glm::tvec3<T> &o, const glm::tvec3<T> &l) {
		auto t = o - c;
		auto lent = glm::length(t);
		auto x = glm::dot(l, t);
		auto d = x*x - lent*lent + r*r;
		if (d < 0)
			return std::numeric_limits<T>::infinity();

		auto l0 = -x + glm::sqrt(d);
		auto l1 = -x - glm::sqrt(d);
		if (l0 < 0 && l1 < 0)
			return std::numeric_limits<T>::infinity();
		if (l0 < 0)
			return l1;
		if (l1 < 0)
			return l0;
		return glm::min(l0, l1);
	}

	T optical_length(const glm::tvec3<T> &P0, const glm::tvec3<T> &P, const T &H, int lut) const {
		auto Vw = P - P0;
		auto lenV = glm::length(Vw);
		if (lenV > 0) Vw /= lenV;

		auto len0 = optical_length_from_infinity(P, Vw, H, lut);
		auto len1 = optical_length_from_infinity(P0, Vw, H, lut);
		auto result = glm::abs(len1 - len0);

		return result;
	}

	T optical_length_from_infinity(const glm::tvec3<T> &P0, const glm::tvec3<T> &L, const T &H, int lut) const {
		auto H_max = ap.max_height(H);

		T h;
		auto Y = P0 - ap.center;
		h = glm::length(Y) - ap.radius;
		Y = glm::normalize(Y);

		auto cos_phi = glm::dot(Y, L);

		auto h_idx = lut_t::height_to_lut_idx(h, H_max);
		auto phi_idx = lut_t::view_zenith_to_lut_idx(cos_phi);
		auto result = final_lut->sample_optical_length_value(lut, { h_idx, phi_idx });

		_assert(result > 0);

		return ap.ro0 * result;
	}


	template <int Chunks, int i>
	void build_optical_length_lut(int lut, const T &H) {
		static_assert(lut_t::optical_length_size % Chunks == 0, "");

		int chunk = lut_t::optical_length_size / Chunks;

		auto H_max = ap.max_height(H);
		for (int x = chunk * i; x<chunk * (i + 1); ++x) {
			for (int y = 0; y<lut_t::optical_length_size; ++y) {
				auto h = lut_t::height_for_lut_idx(static_cast<T>(x) / static_cast<T>(lut_t::optical_length_size - 1), H_max);
				auto phi = lut_t::view_zenith_for_lut_idx(static_cast<T>(y) / static_cast<T>(lut_t::optical_length_size - 1));

				glm::tvec3<T> P0 = ap.center + glm::tvec3<T>{ 0, ap.radius + h, 0 };
				glm::tvec3<T> V = { glm::sin(phi),glm::cos(phi),0 };
				auto l0 = intersect_line_sphere(ap.center, ap.radius + H_max, P0 - decltype(P0){0, 0.1, 0}, V);
				auto l1 = intersect_line_sphere(ap.center, ap.radius, P0 + decltype(P0){0, 0.1, 0}, V);
				auto l = glm::min(l0, l1);

				auto lambda = [&](double step) {
					auto P = P0 + step * V;
					auto hx = glm::length(P - ap.center) - ap.radius;
					return glm::exp(-hx / H);// this->ap.pressure(hx, H);
				};

				auto val = romberg_integration<14>::integrate(lambda, 0, l);
				final_lut->write_optical_length_value(lut, { x,y }, val);
			}
			if (x % 32 == 31)
				std::cout << "x";
		}
	}


	auto scatter_sample(int k, const T &h, const T &cos_phi, const T &cos_delta,
						const glm::tvec3<T> &V, const glm::tvec3<T> &L) const {
		glm::dvec3 idx = { lut_t::height_to_lut_idx(h, Hmax), lut_t::view_zenith_to_lut_idx(cos_phi), lut_t::sun_zenith_to_lut_idx(cos_delta) };
		if (k == 1)
			return final_lut->sample_scatter_value(idx, V, L);
		else
			return sample_last_scatter_value(idx);
	}

	auto scatter_gather(int k, const glm::tvec3<T> &P0, const glm::tvec3<T> &V, const glm::tvec3<T> &L) const {
		static_assert(M > 1, "M should be positive");

		auto Y = P0 - ap.center;
		auto h = glm::length(Y) - ap.radius;
		Y = glm::normalize(Y);
		auto cos_delta = glm::dot(Y, L);

		glm::tvec3<T> Tangent = { 1,0,0 };
		if (glm::abs(glm::dot(Tangent, V)) > .95) Tangent = { 0,1,0 };
		glm::tvec3<T> U = glm::normalize(glm::cross(Tangent, V));
		Tangent = glm::normalize(glm::cross(U, V));
		auto TBN = glm::tmat3x3<T>{ U, V, Tangent };

		auto lambda = [&](double sample_theta, double sample_phi) {
			T sin_theta = glm::sin(sample_theta);
			auto omega = TBN * glm::tvec3<T>{ sin_theta * glm::cos(sample_phi), sin_theta * glm::sin(sample_phi), glm::cos(sample_theta) };

			auto cos_phi = glm::dot(Y, omega);

			auto c = -glm::dot(omega, V);
			auto Fr = rayleigh_phase_function(c);
			auto Fm = cornette_shanks_phase_function(c, ap.phase + .075);

			auto sample = scatter_sample(k, h, cos_phi, cos_delta, omega, L);

			return double_vec3<T>{ Fr * sample, Fm * sample};
		};
		auto normalizer_lambda = [&](double sample_theta, double sample_phi) {
			T sin_theta = glm::sin(sample_theta);
			auto omega = TBN * glm::tvec3<T>{ sin_theta * glm::cos(sample_phi), sin_theta * glm::sin(sample_phi), glm::cos(sample_theta) };

			auto c = -glm::dot(omega, V);
			auto Fr = rayleigh_phase_function(c);
			auto Fm = cornette_shanks_phase_function(c, ap.phase + .075);

			return glm::tvec2<T>{ Fr, Fm };
		};

		auto result = gaussian_quadrature_spherical_integration<M>::integrate(lambda, quadrature_points);
		auto normalizer = gaussian_quadrature_spherical_integration<M>::integrate(normalizer_lambda, quadrature_points);

		result.v0 /= normalizer.x;
		result.v1 /= normalizer.y;

		return double_vec3<T>{ result.v0, result.v1 };
	}

	auto scatter(int k, const T &h0, const glm::tvec3<T> &V, const glm::tvec3<T> &L) const {
		glm::tvec3<T> P0 = ap.center + glm::tvec3<T>{ 0, ap.radius + h0, 0 };

		auto l0 = intersect_line_sphere(ap.center, ap.radius + Hmax, P0 - decltype(P0){0, 0.1, 0}, V);
		auto l1 = intersect_line_sphere(ap.center, ap.radius, P0 + decltype(P0){0, 0.1, 0}, V);
		auto path_length = glm::min(l0, l1);
		if ((h0 == 0 && V.y < 0) ||
			(h0 >= Hmax - 1e-30 && V.y > 0))
			return double_vec3<T>{ { 0, 0, 0 }, { 0,0,0 } };
		_assert(!glm::isnan(path_length) && !glm::isinf(path_length));

		auto Hr = ap.scale_height();
		auto Hm = ap.scale_height_aerosols();

		auto lambda = [&](double step) {
			auto P = P0 + step * V;
			auto h = glm::length(P - ap.center) - ap.radius;
			auto t = ap.rayleigh_extinction_coeffcient() * optical_length(P0, P, Hr, 0) + 
				ap.mie_extinction_coeffcient() * optical_length(P0, P, Hm, 1);

			auto light_occlusion = intersect_line_sphere(ap.center, ap.radius, P + decltype(P){0, .1, 0}, L);
			if (!glm::isinf(light_occlusion))
				return double_vec3<T>{ { 0, 0, 0 }, { 0,0,0 } };

			auto density_r = ap.pressure(h, Hr);
			auto density_m = ap.pressure(h, Hm);
			auto scatter_r = density_r * ap.rayleigh_scattering_coefficient;
			auto scatter_m = density_m * glm::tvec3<T>(ap.mie_scattering_coefficient);

			if (k == 1) {
				// For first order scatter we need to take into account the attenuation from light source
				t += ap.rayleigh_extinction_coeffcient() * optical_length_from_infinity(P, L, Hr, 0) +
					ap.mie_extinction_coeffcient() * optical_length_from_infinity(P, L, Hm, 1);
			}

			// For first order scatter we sample the actual amount of scattered light at point P from the light source.
			// For multiple scattering we sample the gathered scatters across a 4*pi steradians at point P.
			double_vec3<T> result;
			result.v0 = scatter_r;
			result.v1 = scatter_m;

			if (k > 1) {
				auto G = scatter_gather(k - 1, P, V, L);
				result *= G;
			}

			return result * glm::exp(-t);
		};

		double_vec3<T> val;
		if (k == 1)
			val = romberg_integration<N + 3>::integrate(lambda, 0, path_length);
		else
			val = romberg_integration<N>::integrate(lambda, 0, path_length);
		_assert(!glm::any(glm::isnan(val.v0)) && !glm::any(glm::isinf(val.v0)));
		_assert(!glm::any(glm::isnan(val.v1)) && !glm::any(glm::isinf(val.v1)));

		if (k == 1) {
			auto c = -glm::dot(L, V);
			auto Fr = rayleigh_phase_function(c);

			val.v0 = val.v0 * Fr;
		}
		else {
			val.v0 = val.v0 + val.v1;
		}

		return val;
	}

	void scatter(int k, const glm::ivec3 &idx,
				 const T &h, const T &phi, const T &delta) const {
		static_assert(N >= 1, "Expected positive N");
		_assert(k >= 1);

		glm::tvec3<T> V = { glm::sin(phi),glm::cos(phi),0 };
		glm::tvec3<T> L = { glm::sin(delta),glm::cos(delta),0 };

		auto sample = scatter(k, h, V, L);
		auto val = sample.v0;
		if (k == 1)
			final_lut->write_scatter_value(idx, { val, sample.v1 });

		temp_lut->scatter[idx.x][idx.y][idx.z] = val;
	}

	void ambient(const glm::ivec3 &idx,
				 const T &scatter_lut_idx_h, const T &scatter_lut_idx_delta, const T &delta, const T &NdotL) {
		glm::tvec3<T> L = { glm::sin(delta),glm::cos(delta),0 };

		T angle = delta - glm::acos(NdotL);
		glm::tvec3<T> N = { glm::sin(angle),glm::cos(angle),0 };

		_assert(glm::abs(glm::dot(N, L) - NdotL) < 1e-15);

		auto lambda = [&](double sample_theta, double sample_phi) {
			T sin_theta = glm::sin(sample_theta);
			auto omega = glm::tvec3<T>{ sin_theta * glm::cos(sample_phi), sin_theta * glm::sin(sample_phi), glm::cos(sample_theta) };

			auto lambertian = glm::dot(N, omega);
			if (lambertian <= .0)
				return glm::tvec3<T>(0);

			auto cos_phi = omega.y;
			auto scatter_lut_y = lut_t::view_zenith_to_lut_idx(cos_phi);

			auto gamma = glm::dot(omega, L);
			auto decay = cie_scattering_indicatrix(glm::acos(gamma), gamma) / cie_scattering_indicatrix_normalizer();

			auto sample = final_lut->sample_scatter_value({ scatter_lut_idx_h, scatter_lut_y, scatter_lut_idx_delta }, omega, L);

			return lambertian * decay * sample;
		};

		auto result = gaussian_quadrature_spherical_integration<M>::integrate(lambda, quadrature_points);

		final_lut->write_ambient_value(idx, result);
	}

public:
	atmospherics_precompute_scattering(const atmospherics_properties<T> &ap) : ap(ap) {
		final_lut = std::make_unique<lut_t>(ap);
		temp_lut = std::make_unique<temp_lut_t>();
		last_scatter_lut = std::make_unique<temp_lut_t>();

		auto Hmax_r = ap.max_height(ap.scale_height());
		auto Hmax_m = ap.max_height(ap.scale_height_aerosols());
		Hmax = glm::max(Hmax_r, Hmax_m);

		quadrature_points = gaussian_quadrature_spherical_integration<M>::generate_points();
	}

	void build_optical_length_lut_air() {
		auto H = ap.scale_height();

		auto f0 = std::async([this, H]() { build_optical_length_lut<4, 0>(0, H); });
		auto f1 = std::async([this, H]() { build_optical_length_lut<4, 1>(0, H); });
		auto f2 = std::async([this, H]() { build_optical_length_lut<4, 2>(0, H); });
		auto f3 = std::async([this, H]() { build_optical_length_lut<4, 3>(0, H); });

		f0.get();
		f1.get();
		f2.get();
		f3.get();
	}
	void build_optical_length_lut_aerosols() {
		auto H = ap.scale_height_aerosols();

		auto f0 = std::async([this, H]() { build_optical_length_lut<4, 0>(1, H); });
		auto f1 = std::async([this, H]() { build_optical_length_lut<4, 1>(1, H); });
		auto f2 = std::async([this, H]() { build_optical_length_lut<4, 2>(1, H); });
		auto f3 = std::async([this, H]() { build_optical_length_lut<4, 3>(1, H); });

		f0.get();
		f1.get();
		f2.get();
		f3.get();
	}
	void build_optical_length_lut() {
		std::cout << "Building Optical Length LUT..." << std::endl;

		build_optical_length_lut_air();
		build_optical_length_lut_aerosols();

		std::cout << std::endl << "Done" << std::endl;
	}

	template <int Chunks, int i>
	void build_scatter_lut_worker(int k) {
		static_assert(lut_t::scatter_size0 % Chunks == 0, "");

		int chunk = lut_t::scatter_size0 / Chunks;
		for (int x = chunk * i; x<chunk*(i+1); ++x) {
			for (int y = 0; y<lut_t::scatter_size1; ++y) {
				for (int z = 0; z<lut_t::scatter_size2; ++z) {
					auto h = lut_t::height_for_lut_idx(static_cast<T>(x) / static_cast<T>(lut_t::scatter_size0 - 1), Hmax);
					auto phi = lut_t::view_zenith_for_lut_idx(static_cast<T>(y) / static_cast<T>(lut_t::scatter_size1 - 1));
					auto delta = lut_t::sun_zenith_for_lut_idx(static_cast<T>(z) / static_cast<T>(lut_t::scatter_size2 - 1));

					scatter(k, { x,y,z }, h, phi, delta);
				}

				if (y % (lut_t::scatter_size1>>2) == (lut_t::scatter_size1>>2) - 1)
					std::cout << "x";
			}
		}
	}

	void build_scatter_lut(int k_end) {
		assert(k_end >= 1);

		std::cout << std::endl << "Building scatter LUT..." << std::endl;

		for (int i=1; i<=k_end; ++i) {
			auto f0 = std::async([this, i]() { build_scatter_lut_worker<4, 0>(i); });
			auto f1 = std::async([this, i]() { build_scatter_lut_worker<4, 1>(i); });
			auto f2 = std::async([this, i]() { build_scatter_lut_worker<4, 2>(i); });
			auto f3 = std::async([this, i]() { build_scatter_lut_worker<4, 3>(i); });

			f0.get();
			f1.get();
			f2.get();
			f3.get();

			std::cout << std::endl << "Scatter index " << i << " done." << std::endl;

			double_vec3<T> max = { { 0,0,0 },{0,0,0} };
			T max_len = T(0);
			for (int x = 0; x < lut_t::scatter_size0; ++x)
				for (int y = 0; y < lut_t::scatter_size1; ++y)
					for (int z = 0; z < lut_t::scatter_size2; ++z) {
						auto val = temp_lut->scatter[x][y][z];

						if (i > 1)
							final_lut->add_scatter_value({ x,y,z }, val);

						auto l = glm::length(val);
						if (l > max_len) {
							max = { val,final_lut->read_scatter_value({x,y,z}).v1 };
							max_len = l;
						}
					}
			std::cout.precision(17);
			std::cout << "Max: Multiple - <" << max.v0.x << ", " << max.v0.y << ", " << max.v0.z << ">" << std::endl <<
						 "     Mie0 - <" << max.v1.x << ", " << max.v1.y << ", " << max.v1.z << ">" << std::endl;

			std::swap(temp_lut, last_scatter_lut);

			std::cout << std::endl;
		}

		final_lut->set_scatter_index(k_end);

		std::cout << "Done" << std::endl;
	}

	template <int Chunks, int i>
	void build_ambient_lut_worker() {
		static_assert(lut_t::ambient_size_0 % Chunks == 0, "");

		int chunk = lut_t::ambient_size_0 / Chunks;
		for (int x = chunk * i; x < chunk*(i + 1); ++x) {
			for (int y = 0; y < lut_t::ambient_size_1; ++y) {
				for (int z = 0; z < lut_t::ambient_size_2; ++z) {
					auto scatter_lut_idx_h = static_cast<T>(x) / static_cast<T>(lut_t::ambient_size_0 - 1);
					auto scatter_lut_idx_delta = static_cast<T>(y) / static_cast<T>(lut_t::ambient_size_1 - 1);
					auto delta = lut_t::sun_zenith_for_lut_idx(scatter_lut_idx_delta);
					auto NdotL = lut_t::ambient_NdotL_for_lut_idx(static_cast<T>(z) / static_cast<T>(lut_t::ambient_size_2 - 1));

					ambient({ x,y,z }, scatter_lut_idx_h, scatter_lut_idx_delta, delta, NdotL);
				}

				if (y % (lut_t::ambient_size_1) == (lut_t::ambient_size_1) - 1)
					std::cout << "x";
			}
		}
	}

	void build_ambient_lut() {
		std::cout << std::endl << "Building ambient LUT..." << std::endl;

		auto f0 = std::async([this]() { build_ambient_lut_worker<4, 0>(); });
		auto f1 = std::async([this]() { build_ambient_lut_worker<4, 1>(); });
		auto f2 = std::async([this]() { build_ambient_lut_worker<4, 2>(); });
		auto f3 = std::async([this]() { build_ambient_lut_worker<4, 3>(); });

		f0.get();
		f1.get();
		f2.get();
		f3.get();

		std::cout << "Done" << std::endl;
	}

	void load(const std::string &path) {
		final_lut->load(path);
	}

	void write_out(const std::string &path) const {
		std::cout << "Writing final LUT..." << std::endl;
		final_lut->write_out(path);
	}
};

}
}
