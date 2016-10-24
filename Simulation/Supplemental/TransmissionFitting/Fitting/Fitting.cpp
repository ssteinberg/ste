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

struct transmission_fit_data {
	// m*((erf(a*x - b) + 1) + c*x) + d
	double a, b, c, d, m;
};

struct transmission_fit {
	static constexpr int N = 256;

	unsigned char ndf_type[8];
	std::uint16_t version;
	std::uint16_t size;
	std::uint32_t hash;

	std::array<std::array<transmission_fit_data, N>, N> data;
};

using fitting_future = std::future<std::tuple<std::string, int, int>>;

bool matlab_eval(const std::string &cmd, int x, int y, int elements, Engine *matlabEngine, transmission_fit *lut, double *rmse) {
	try {
		if (engEvalString(matlabEngine, cmd.c_str()) != 0)
			return false;

		auto *rmsexp = engGetVariable(matlabEngine, "rmse");
		*rmse = *reinterpret_cast<const double*>(mxGetData(rmsexp));
		mxDestroyArray(rmsexp);

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
		double rmse;
		if (matlab_eval(cmd, x, y, 5, matlabEngine, lut, &rmse)) {
			std::cout << "<" << x << "," << y << ">: RMSE: " << rmse << "   - Fit: " << lut->data[x][y].m << "*((erf(" << lut->data[x][y].a << "*x - " << lut->data[x][y].b << ") + 1) + " << lut->data[x][y].c << "*x) + " << lut->data[x][y].d << std::endl;
			if (rmse > .075) {
				std::cout << "Bad fit!" << std::endl;
			}
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

bool V(const glm::dvec3 &m, const glm::dvec3 &l) {
	double cos_gamma = glm::dot(m, l);
	return cos_gamma > 0;
}

double transmission_fresnel(const glm::dvec3 &m, const glm::dvec3 &l, double r) {
	double cosx = glm::dot(l, m);

	double sin_critical = glm::min<double>(1.0, r);
	double cos_critical = 1. - sin_critical * sin_critical;
	if (cosx * cosx <= cos_critical * cos_critical)
		return .0;

	auto sinx2 = std::complex<double>(1) - cosx * cosx;
	auto c = std::sqrt(std::complex<double>(1) - sinx2 / (r*r));

	auto Rp = std::norm((cosx - r * c) / (cosx + r * c));
	auto Rs = std::norm((c - r * cosx) / (r * cosx + c));

	return 1. - glm::clamp(.5 * (Rp + Rs), .0, 1.);
}

double D(double cos_theta, double roughness) {
	double a = roughness * roughness;
	double t = (a*a - 1.) * cos_theta*cos_theta + 1.;
	return a*a * glm::one_over_pi<double>() / (t*t);
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
	constexpr double cos_theta_step = 1.0 / 100.;
	for (int x = 0; x<transmission_fit::N; ++x) {
		for (int y = 0; y<transmission_fit::N; ++y) {
			double ior_ratio = static_cast<double>(x) / static_cast<double>(transmission_fit::N - 1) * (Rmax - Rmin) + Rmin;
			double roughness = static_cast<double>(y) / static_cast<double>(transmission_fit::N - 1);
			roughness = glm::max<double>(roughness, 1e-4);

			double omega = glm::asin(glm::min<double>(1.0, ior_ratio));

			std::cout << "Integrating - r: " << ior_ratio << ", roughness: " << roughness << std::endl;

			futures[it % 2] = std::async([=]() {
				std::string strx, stry;
				for (double cos_theta = .0; cos_theta <= 1.0; 
					 cos_theta = (cos_theta < 1 && cos_theta + cos_theta_step >= 1.) ? 1. : cos_theta + cos_theta_step) {
					if (strx.length() > 0) {
						strx += " ";
						stry += " ";
					}

					double theta = glm::acos(cos_theta);

					auto f = [&](double x, bool with_fresnel) {
						auto g = [&](double theta_v, double theta, double phi) {
							glm::dvec3 m = { glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta) };
							glm::dvec3 l = { glm::sin(theta_v), 0, glm::cos(theta_v) };

							if (!V(m, l))
								return .0;

							return with_fresnel ? transmission_fresnel(m, l, ior_ratio) : 1.0;
						};

						auto intg = StE::romberg_integration<10>::integrate(std::bind(g, theta, x, std::placeholders::_1), 0, glm::two_pi<double>());
						auto c = .5 * glm::sin(2 * x) * D(glm::cos(x), roughness);

						return c * intg;
					};

					double s = 0.;
					double t = glm::half_pi<double>();

					double numerical_integration = StE::romberg_integration<10>::integrate(std::bind(f, std::placeholders::_1, true), s, t);
					double normalizer = StE::romberg_integration<10>::integrate(std::bind(f, std::placeholders::_1, false), s, t);

					double res = normalizer > 0 ? numerical_integration / normalizer : .0;
					if (std::isnan(res)) {
						std::cout << "!! nan for " << omega <<  "," << roughness << " !!" << std::endl;
						res = 1.0;
					}

					strx += std::to_string(cos_theta);
					stry += std::to_string(res);
				}

				std::string cmd = "x=[" + strx + "];\ny=[" + stry + R"(];
						w = ones(length(x),1); w(1) = 10; w(length(x))=100;

						[xData, yData, weights] = prepareCurveData( x, y, w );

						ft = fittype( 'm*((erf(a*x - b) + 1) + c*x) + d', 'independent', 'x', 'dependent', 'y' );
						opts = fitoptions( 'Method', 'NonlinearLeastSquares' );
						opts.Display = 'Off';
						opts.Lower = [-Inf -Inf 0 -Inf 0];
						opts.MaxFunEvals = 6000;
						opts.MaxIter = 4000;
						opts.StartPoint = [0 0 0 0 .5];
						opts.TolFun = 1e-07;
						opts.Upper = [Inf Inf 1 Inf 1];
						opts.Weights = weights;

						[sf, sg] = fit(xData, yData, ft, opts);

						p = coeffvalues(sf);
						rmse = sg.rmse;
					)";

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
	lut->version = 4;

	boost::crc_32_type crc_computer;
	crc_computer.process_bytes(reinterpret_cast<const std::uint8_t*>(&lut->data), sizeof(lut->data));
	lut->hash = crc_computer.checksum();

	std::ofstream ofs("microfacet_ggx_transmission_fit.bin", std::ios::binary);
	ofs.write(reinterpret_cast<const char*>(lut), sizeof(*lut));
	ofs.close();

	delete lut;

	return 0;
}
