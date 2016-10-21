// Fitting.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "engine.h"

#define GLM_FORCE_AVX
#define GLM_EXT_INCLUDED
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/bit.hpp>

#include <boost/crc.hpp>

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cmath>
#include <future>
#include <complex>

namespace StE {

/**
*	@brief	Implementation of Romberg's method for numerical integration
*/
template <unsigned N>
class romberg_integration {
	static_assert(N > 0, "Iterations count must be positive.");

private:
	// Calculates the partial sum of trapezoids using half of n equally spaced segments between a and b.
	template <typename F>
	static double trapezoid(const F &f, double h, double a, double b, std::uint64_t n) {
		double t = .0;

		for (std::uint64_t i = 1; i<n; i += 2) {
			double x = a + static_cast<double>(i) * h;
			t += f(x);
		}

		return t;
	}

public:
	/**
	*	@brief	Evaluate definite integral
	*
	*	https://en.wikipedia.org/wiki/Romberg's_method
	*
	* 	@param f	Function to integrate
	* 	@param a	Interval start
	* 	@param b	Interval end
	*/
	template <typename F>
	static double integrate(const F &f, double a, double b) {
		using T = double;

		if (a >= b)
			return T(.0);

		T t;
		std::array<T, N> Tk;
		Tk.fill(T(.0));

		Tk[0] = .5 * (b - a) * (f(a) + f(b));

		for (std::uint64_t n = 1; n < N; ++n) {
			std::uint64_t k = static_cast<std::uint64_t>(1) << n;
			double h = (b - a) / static_cast<double>(k);
			t = .5 * Tk[0] + h * trapezoid(f, h, a, b, k);

			for (std::uint64_t m = 1; m <= n; ++m) {
				T s = t + (t - Tk[m - 1]) / static_cast<double>((1ull << 2ull * m) - 1);
				Tk[m - 1] = t;
				t = s;
			}

			Tk[n] = t;
		}

		return static_cast<T>(Tk[N - 1]);
	}
};

}

double transmission_fresnel(double theta_v, double theta, double phi, double r) {
	glm::dvec3 m = { glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta) };
	glm::dvec3 l = { glm::sin(theta_v), 0, glm::cos(theta_v) };

	std::complex<double> cosx = glm::dot(l, m);
	auto sinx2 = std::complex<double>(1) - cosx * cosx;
	auto c = std::sqrt(std::complex<double>(1) - sinx2 / (r*r));

	auto Rp = std::norm((cosx - r * c) / (cosx + r * c));
	auto Rs = std::norm((r * cosx - c) / (r * cosx + c));

	return 1. - .5 * (Rp + Rs);
}

struct transmission_fit_data {
	double a1, b1, c1;
	double a2, b2, c2;
};

struct transmission_fit {
	static constexpr int N = 512;

	unsigned char ndf_type[8];
	std::uint16_t version;
	std::uint16_t size;
	std::uint32_t hash;

	std::array<std::array<transmission_fit_data, N>, N> data;
};

using fitting_future = std::future<std::tuple<std::string, int, int>>;

bool matlab_eval(const std::string &cmd, int x, int y, int elements, Engine *matlabEngine, transmission_fit *lut) {
	try {
		if (engEvalString(matlabEngine, cmd.c_str()) != 0)
			return false;

		auto *mxp = engGetVariable(matlabEngine, "p");
		if (mxGetNumberOfElements(mxp) != elements) {
			mxDestroyArray(mxp);
			return false;
		}

		const double *p = reinterpret_cast<const double*>(mxGetData(mxp));
		std::copy(p, p + elements, reinterpret_cast<double*>(&lut->data[x][y]));
		mxDestroyArray(mxp);
	}
	catch (std::exception) {
		return false;
	}

	return true;
}

void fit(fitting_future &future, Engine* &matlabEngine, transmission_fit *lut) {
	if (!future.valid())
		return;

	auto fdata = future.get();

	auto cmd = std::get<0>(fdata);
	int x = std::get<1>(fdata);
	int y = std::get<2>(fdata);

	for (int itry = 0; itry < 3; ++itry) {
		if (matlab_eval(cmd, x, y, 6, matlabEngine, lut)) {
			std::cout << "gauss2(" << x << "," << y << "): <" << lut->data[x][y].a1 << ", " << lut->data[x][y].b1 << ", " << lut->data[x][y].c1 << "; " <<
				lut->data[x][y].a2 << ", " << lut->data[x][y].b2 << ", " << lut->data[x][y].c2 << ">" << std::endl;
			return;
		}

		std::cout << "Matlab fitting error... Retrying..." << std::endl;
		try {
			engClose(matlabEngine);
		}
		catch (std::exception) {}

		void * vpDcom = nullptr;
		int ret = 0;
		matlabEngine = engOpenSingleUse(0, vpDcom, &ret);
	}

	throw new std::runtime_error("Couldn't restart Matlab");
}

int main() {
	constexpr double Rmin = .2;
	constexpr double Rmax = 3.2;
	auto *lut = new transmission_fit;

	void * vpDcom = nullptr;
	int ret = 0;
	Engine* matlabEngine = engOpenSingleUse(0, vpDcom, &ret);

	fitting_future futures[2];

	int it = 0;
	constexpr double cos_theta_step = 1.0 / 80.0;
	for (int x = 0; x<transmission_fit::N; ++x) {
		for (int y = 0; y<transmission_fit::N; ++y) {
			double ior_ratio = static_cast<double>(x) / static_cast<double>(transmission_fit::N - 1) * (Rmax - Rmin) + Rmin;
			double roughness = static_cast<double>(y) / static_cast<double>(transmission_fit::N - 1);

			double omega = glm::asin(glm::min<double>(1.0, ior_ratio));

			std::cout << "Integrating - r: " << ior_ratio << ", roughness: " << roughness << std::endl;

			futures[it % 2] = std::async([=]() {
				std::string strx, stry;
				for (double cos_theta = .0; cos_theta <= 1.0;) {
					if (strx.length() > 0) {
						strx += " ";
						stry += " ";
					}

					double theta = glm::acos(cos_theta);

					auto f = [&](double x, double c, bool with_fresnel) {
						// Dirac delta when roughness == 0
						if (roughness == .0) {
							auto f0 = (1 - ior_ratio) / (1 + ior_ratio);
							return glm::abs(x-theta) < 1e-6 ? f0*f0 : .0;
						}

						double a = roughness * roughness;
						double t = c * sin(glm::pi<double>() / 2 * (x - (theta - c)) / c);
						auto intg = with_fresnel ?
							StE::romberg_integration<9>::integrate(std::bind(transmission_fresnel, theta, x, std::placeholders::_1, ior_ratio), -t, +t) :
							1.0;

						double cx = glm::cos(x);
						double denom = cx * cx * (a*a - 1.0) + 1.0;
						return a*a / glm::pi<double>() * .5 * glm::sin(2 * glm::abs(x)) / (denom * denom) * intg;
					};

					double res = .0;
					if (omega > .0) {
						double s = glm::max(theta - omega, -glm::half_pi<double>());
						double t = glm::min(theta + omega, glm::half_pi<double>());
						double norm_s = glm::max(theta - glm::half_pi<double>(), -glm::half_pi<double>());
						double norm_t = glm::min(theta + glm::half_pi<double>(), glm::half_pi<double>());

						double numerical_integration = StE::romberg_integration<10>::integrate(std::bind(f, std::placeholders::_1, omega, true), s, t);
						double normalizer = StE::romberg_integration<10>::integrate(std::bind(f, std::placeholders::_1, glm::half_pi<double>(), false), 
							norm_s, norm_t);

						res = normalizer > 0 ? numerical_integration / normalizer : .0;
						if (std::isnan(res)) {
							std::cout << "!! nan for " << omega<<  "," << roughness << " !!" << std::endl;
							res = 1.0;
						}
					}

					strx += std::to_string(cos_theta);
					stry += std::to_string(res);

					cos_theta += cos_theta_step;
				}

				std::string cmd = "x=[" + strx + "];\ny=[" + stry + "];\nw = ones(length(x),1); w(1) = 10; w(length(x))=100;ft = fitoptions('gauss2'); ft.Weights = w; sf = fit(x.',y.', 'gauss2', ft);\np = coeffvalues(sf);";//plot(sf,x,y,'o');";

				return std::make_tuple(cmd, x, y);
			});

			++it;
			fit(futures[it % 2], matlabEngine, lut);

			std::cout << "iteration: " << std::to_string(it) << " (" << std::to_string(100.0*static_cast<double>(it) / static_cast<double>(transmission_fit::N*transmission_fit::N)) << "%)" << std::endl;
		}
	}

	++it;
	fit(futures[it % 2], matlabEngine, lut);

	engClose(matlabEngine);

	memset(lut->ndf_type, 0, sizeof(lut->ndf_type));
	lut->ndf_type[0] = 'G'; lut->ndf_type[1] = 'G'; lut->ndf_type[2] = 'X';
	lut->size = transmission_fit::N;
	lut->version = 3;

	boost::crc_32_type crc_computer;
	crc_computer.process_bytes(reinterpret_cast<const std::uint8_t*>(&lut->data), sizeof(lut->data));
	lut->hash = crc_computer.checksum();

	std::ofstream ofs("microfacet_ggx_transmission_fit.bin", std::ios::binary);
	ofs.write(reinterpret_cast<const char*>(lut), sizeof(*lut));
	ofs.close();

	delete lut;

	return 0;
}
