// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.h"
#include "atmospheric_properties.hpp"
#include "phase.hpp"

#include "romberg_integration.hpp"

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

namespace _detail {

template<typename T>
class _atmospherics_precompute_scattering_data {
public:
	static constexpr int optical_length_size = 2048;
	static constexpr int scatter_size0 = 128;
	static constexpr int scatter_size1 = 384;
	static constexpr int scatter_size2 = 196;
	//static constexpr int scatter_size3 = 32;

	using scatter_element = glm::tvec3<T>;

	using optical_length_lut_t = T[optical_length_size][optical_length_size];
	using scatter_lut_t = scatter_element[scatter_size0][scatter_size1][scatter_size2];// [scatter_size3];
	using mie_single_scatter_lut_t = T[scatter_size0][scatter_size1][scatter_size2];// [scatter_size3];

private:
	mutable unsigned char type[8];
	std::uint16_t version{ 3 };
	std::uint8_t  sizeof_scalar{ sizeof(T) };
	std::uint32_t optical_length_dims{ optical_length_size };
	std::uint32_t scatter_dims_0{ scatter_size0 };
	std::uint32_t scatter_dims_1{ scatter_size1 };
	std::uint32_t scatter_dims_2{ scatter_size2 };
	mutable std::uint32_t hash{ 0 };

	atmospherics_properties<T> ap;

	struct {
		optical_length_lut_t optical_length[2];
		scatter_lut_t multi_scatter;
		mie_single_scatter_lut_t mie_single_scatter;
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

public:
	_atmospherics_precompute_scattering_data(const atmospherics_properties<T> &ap) : ap(ap) {}

	void write_multi_scatter_value(const glm::ivec3 &idx, const glm::tvec3<T> &ms) {
		data.multi_scatter[idx.x][idx.y][idx.z] = ms;
	}
	void write_m0_scatter_value(const glm::ivec3 &idx, const T &m0) {
		data.mie_single_scatter[idx.x][idx.y][idx.z] = m0;
	}
	void write_optical_length_value(int lut, const glm::ivec2 &idx, const T &value) {
		data.optical_length[lut][idx.x][idx.y] = value;
	}
	auto sample_optical_length_value(int lut, const glm::dvec2 &idx) const {
		auto x = glm::floor(idx * static_cast<double>(optical_length_size - 1));
		auto f = glm::fract(idx);

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

	auto &optical_length_lut(int lut) { return data.optical_length[lut]; }
	auto &multi_scatter_lut() { return data.multi_scatter; }
	auto &m0_scatter_lut() { return data.mie_single_scatter; }

	void load(const std::string &path) {
		std::ifstream ifs(path, std::ios::binary);
		ifs.read(reinterpret_cast<char*>(this), sizeof(*this));
		ifs.close();

		boost::crc_32_type crc_computer;
		crc_computer.process_bytes(reinterpret_cast<const std::uint8_t*>(&data), sizeof(data));
		_assert(this->hash == crc_computer.checksum());
	}

	void write_out(const std::string &path) const {
		memcpy(this->type, "SCATTER  ", 8);

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

	struct temp_lut_t {
		lut_t::scatter_lut_t scatter;
	};
	struct m0_phase_lut_t {
		lut_t::mie_single_scatter_lut_t m0;
	};

private:
	atmospherics_properties<T> ap;
	T Hmax;

	std::unique_ptr<lut_t> final_lut;
	std::unique_ptr<m0_phase_lut_t> m0_phase_lut;
	std::unique_ptr<temp_lut_t> luts0;
	std::unique_ptr<temp_lut_t> luts1;

	auto sample_scatter_value(const lut_t::scatter_lut_t &lut, const glm::dvec3 &idx) const {
		auto x = glm::floor(idx * glm::dvec3{ lut_t::scatter_size0 - 1,lut_t::scatter_size1 - 1,lut_t::scatter_size2 - 1 });
		auto f = glm::fract(idx);

		lut_t::scatter_element s[2][2][2];
		for (int i1 = 0; i1<2; ++i1)
			for (int i2 = 0; i2<2; ++i2)
				for (int i3 = 0; i3 < 2; ++i3) {
					s[i1][i2][i3] = lut[glm::min<int>(lut_t::scatter_size0 - 1, static_cast<int>(x.x) + i1)]
					[glm::min<int>(lut_t::scatter_size1 - 1, static_cast<int>(x.y) + i2)]
					[glm::min<int>(lut_t::scatter_size2 - 1, static_cast<int>(x.z) + i3)];
				}

		for (int i1 = 0; i1<2; ++i1)
			for (int i2 = 0; i2 < 2; ++i2) {
				s[0][i1][i2] = glm::mix(s[0][i1][i2], s[1][i1][i2], f.x);
			}
		for (int i1 = 0; i1 < 2; ++i1) {
			s[0][0][i1] = glm::mix(s[0][0][i1], s[0][1][i1], f.y);
		}

		lut_t::scatter_element result;
		result = glm::mix(s[0][0][0], s[0][0][1], f.z);
		return result;
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

	glm::tmat3x3<T> extract_tbn(const glm::tvec3<T> &P0, T &h) const {
		auto Y = P0 - ap.center;
		h = glm::length(Y) - ap.radius;
		Y = glm::normalize(Y);

		auto X = glm::tvec3<T>{ 1,0,0 };
		if (glm::abs(glm::dot(Y, X)) > .95)
			X = { 0,0,1 };
		auto Z = glm::cross(X, Y);
		X = glm::cross(Y, Z);

		return glm::tmat3x3<T>(X, Z, Y);
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
		//auto TBN = extract_tbn(P0, h);

		//auto V = TBN * L;
		auto cos_phi = glm::dot(Y, L);

		auto h_idx = lut_t::height_to_lut_idx(h, H_max);
		auto phi_idx = lut_t::view_zenith_to_lut_idx(cos_phi);
		auto result = final_lut->sample_optical_length_value(lut, { h_idx, phi_idx });

		return ap.ro0 * result;
	}


	void build_optical_length_lut(int lut, const T &H) {
		auto H_max = ap.max_height(H);
		for (int x = 0; x<lut_t::optical_length_size; ++x) {
			for (int y = 0; y<lut_t::optical_length_size; ++y) {
				auto h = lut_t::height_for_lut_idx(static_cast<T>(x) / static_cast<T>(lut_t::optical_length_size), H_max);
				auto phi = lut_t::view_zenith_for_lut_idx(static_cast<T>(y) / static_cast<T>(lut_t::optical_length_size));

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

				auto val = romberg_integration<15>::integrate(lambda, 0, l);
				_assert(!glm::isnan(val) && !glm::isinf(val) && val > 0);
				final_lut->write_optical_length_value(lut, { x,y }, val);
			}
			if (x % 32 == 0)
				std::cout << "x";
		}
	}


	template <int N, int  M, typename T2>
	glm::tvec3<T> scatter(int k, const T &h0, const T &phi, const T &delta,
						  const T2 &scatter_coefficient, const T2 &extinction_coefficient,
						  const T &H, int optical_length_lut,
						  const std::function<T(const T &)> &Fphase, T &F) const {
		glm::tvec3<T> P0 = ap.center + glm::tvec3<T>{ 0, ap.radius + h0, 0 };
		glm::tvec3<T> V = { glm::sin(phi),glm::cos(phi),0 };
		glm::tvec3<T> L = glm::tvec3<T>{ glm::sin(delta),glm::cos(delta),0 };

		auto l0 = intersect_line_sphere(ap.center, ap.radius + Hmax, P0 - decltype(P0){0, 0.1, 0}, V);
		auto l1 = intersect_line_sphere(ap.center, ap.radius, P0 + decltype(P0){0, 0.1, 0}, V);
		auto path_length = glm::min(l0, l1);
		if ((h0 == 0 && V.y < 0) ||
			(h0 >= Hmax - 1e-30 && V.y > 0))
			path_length = 0;
		_assert(!glm::isnan(path_length) && !glm::isinf(path_length));

		auto light_occlusion = intersect_line_sphere(ap.center, ap.radius, P0 + decltype(P0){0,1,0}, L);

		if (path_length == 0 || !glm::isinf(light_occlusion))
			return{ 0,0,0 };

		auto cos_theta = -glm::dot(V, L);
		F = Fphase(cos_theta);

		auto lambda = [&](double step) {
			auto P = P0 + step * V;
			auto h = glm::length(P - ap.center) - ap.radius;
			auto scatter = scatter_coefficient * ap.pressure(h, H);
			auto t = extinction_coefficient * optical_length(P0, P, H, optical_length_lut);

			if (k == 1) {
				// For first order scatter we need to take into account the attenuation from light source
				auto t2 = extinction_coefficient * optical_length_from_infinity(P, L, H, optical_length_lut);
				t += t2;
			}

			// For first order scatter we sample the actual amount of scattered light at point P from the light source.
			// For multiple scattering we sample the gathered scatters across a 4*pi steradians at point P.
			auto sample = glm::tvec3<T>(scatter * glm::exp(-t));
			if (k > 1) {
				auto G = scatter_gather<N, M>(k - 1, P, V, L,
											  Fphase);
				sample *= G;
			}

			_assert(!glm::any(glm::isnan(sample)) && !glm::any(glm::isinf(sample)));

			return sample;
		};

		auto val = romberg_integration<N>::integrate(lambda, 0, path_length);
		_assert(!glm::any(glm::isnan(val)) && !glm::any(glm::isinf(val)));

		return val;
	}

	auto scatter_sample(const T &h, const T &cos_phi, const T &cos_delta) const {
		glm::dvec3 idx = { lut_t::height_to_lut_idx(h, Hmax), lut_t::view_zenith_to_lut_idx(cos_phi), lut_t::sun_zenith_to_lut_idx(cos_delta) };
		return sample_scatter_value(luts0->scatter, idx);
	}

	template <int N, int M>
	auto scatter_gather(int k, const glm::tvec3<T> &P0, const glm::tvec3<T> &V, const glm::tvec3<T> &L,
						const std::function<T(const T &)> &Fphase) const {
		static_assert(M > 1, "M should be positive");
		static_assert(M % 2 == 1, "M should be odd");

		//T h;
		//auto TBN = extract_tbn(P0, h);
		//auto Ll = TBN * L;
		auto Y = P0 - ap.center;
		auto h = glm::length(Y) - ap.radius;
		Y = glm::normalize(Y);
		auto cos_delta = glm::dot(Y, L);

		auto result = glm::tvec3<T>(static_cast<T>(0));

		int w = 0;
		for (int i = 0; i < M; ++i) {
			T sample_theta = static_cast<T>(i) / static_cast<T>(M - 1) * glm::pi<T>();
			T sin_theta = glm::sin(sample_theta);

			int longtitude_samples = i == 0 || i == M - 1 ?
				1 :
				glm::max(3, static_cast<int>(glm::round(sin_theta * static_cast<T>(M << 1))));
			for (int j = 0; j < longtitude_samples; ++j) {
				T sample_phi = static_cast<T>(j << 1) / static_cast<T>(longtitude_samples) * glm::pi<T>();
				glm::tvec3<T> omega = { sin_theta * glm::cos(sample_phi), sin_theta * glm::sin(sample_phi), glm::cos(sample_theta) };

				auto cos_phi = glm::dot(Y, omega);

				auto sample = scatter_sample(h, cos_phi, cos_delta);
				_assert(!glm::any(glm::isnan(sample)) && !glm::any(glm::isinf(sample)));

				auto F = -Fphase(glm::dot(omega, V));

				result += F * sample;
				++w;
			}
		}

		return result / static_cast<T>(w);
	}

	template <int N = 7, int M = 13>
	void scatter(int k, const glm::ivec3 &idx,
				 const T &h, const T &phi, const T &delta) const {
		static_assert(N >= 1, "Expected positive N");
		_assert(k >= 1);

		auto Hr = ap.scale_height();
		auto Hm = ap.scale_height_aerosols();
		T Fr, Fm;

		auto r = scatter<N, M>(k, h, phi, delta,
							   this->ap.rayleigh_scattering_coefficient, this->ap.rayleigh_extinction_coeffcient(),
							   Hr, 0,
							   [](const T &c) { return rayleigh_phase_function(c); }, Fr);
		auto m = scatter<N, M>(k, h, phi, delta,
							   this->ap.mie_scattering_coefficient, this->ap.mie_extinction_coeffcient(),
							   Hm, 1,
							   [this](const T &c) { return cornette_shanks_phase_function(c, this->ap.phase); }, Fm);

		_assert(!glm::any(glm::isnan(r)) && !glm::any(glm::isnan(m)));
		_assert(!glm::any(glm::isinf(r)) && !glm::any(glm::isinf(m)));

		glm::tvec3<T> result = r * Fr + m * Fm;
		if (k == 1) {
			_assert(m.x == m.y && m.x == m.z);
			final_lut->write_m0_scatter_value(idx, m.x);
			m0_phase_lut->m0[idx.x][idx.y][idx.z] = m.x * Fm;
		}
		else {
			result += luts0->scatter[idx.x][idx.y][idx.z];
		}

		luts1->scatter[idx.x][idx.y][idx.z] = result;
	}

public:
	atmospherics_precompute_scattering(const atmospherics_properties<T> &ap) : ap(ap) {
		final_lut = std::make_unique<lut_t>(ap);
		m0_phase_lut = std::make_unique<m0_phase_lut_t>();
		luts0 = std::make_unique<temp_lut_t>();
		luts1 = std::make_unique<temp_lut_t>();

		auto Hmax_r = ap.max_height(ap.scale_height());
		auto Hmax_m = ap.max_height(ap.scale_height_aerosols());
		Hmax = glm::max(Hmax_r, Hmax_m);
	}

	void build_optical_length_lut_air() {
		auto H = ap.scale_height();
		build_optical_length_lut(0, H);
	}
	void build_optical_length_lut_aerosols() {
		auto H = ap.scale_height_aerosols();
		build_optical_length_lut(1, H);
	}
	void build_optical_length_lut() {
		std::cout << "Building Optical Length LUT..." << std::endl;

		auto f0 = std::async([this]() { build_optical_length_lut_air(); });
		auto f1 = std::async([this]() { build_optical_length_lut_aerosols(); });
		f0.get();
		f1.get();

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

					scatter<>(k, { x,y,z }, h, phi, delta);
				}
			}
			std::cout << "x";
		}
	}

	void build_scatter_lut(int k) {
		std::cout << std::endl << "Building scatter LUT..." << std::endl;

		for (int i=1; i<=k; ++i) {
			auto f0 = std::async([this, i]() { build_scatter_lut_worker<4, 0>(i); });
			auto f1 = std::async([this, i]() { build_scatter_lut_worker<4, 1>(i); });
			auto f2 = std::async([this, i]() { build_scatter_lut_worker<4, 2>(i); });
			auto f3 = std::async([this, i]() { build_scatter_lut_worker<4, 3>(i); });

			f0.get();
			f1.get();
			f2.get();
			f3.get();

			std::cout << std::endl << "Scatter index " << i << " done." << std::endl;

			glm::tvec3<T> max = { 0,0,0 };
			for (int x = 0; x < lut_t::scatter_size0; ++x)
				for (int y = 0; y < lut_t::scatter_size1; ++y)
					for (int z = 0; z < lut_t::scatter_size2; ++z)
						max = glm::max(max, luts1->scatter[x][y][z]);
			std::cout.precision(17);
			std::cout << "Max: <" << max.x << ", " << max.y << ", " << max.z << ">" << std::endl << std::endl;

			std::swap(luts0, luts1);
			luts1 = std::make_unique<temp_lut_t>();
		}

		std::cout << "Cleaning M0 scatter from LUT..." << std::endl;
		for (int x = 0; x < lut_t::scatter_size0; ++x) 
			for (int y = 0; y < lut_t::scatter_size1; ++y) 
				for (int z = 0; z < lut_t::scatter_size2; ++z) 
					luts0->scatter[x][y][z] -= m0_phase_lut->m0[x][y][z];

		std::cout << "Writing final LUT..." << std::endl;
		for (int x = 0; x < lut_t::scatter_size0; ++x)
			for (int y = 0; y < lut_t::scatter_size1; ++y)
				for (int z = 0; z < lut_t::scatter_size2; ++z) 
					final_lut->write_multi_scatter_value({ x,y,z }, luts0->scatter[x][y][z]);

		std::cout << "Done" << std::endl;
	}

	void load(const std::string &path) {
		final_lut->load(path);
	}

	void write_out(const std::string &path) const {
		final_lut->write_out(path);
	}
};

}
}
