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
	static glm::dvec3 trapezoid(const F &f, double h, double a, double b, std::uint64_t n) {
		glm::dvec3 t = glm::dvec3(.0);

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
	static glm::dvec3 integrate(const F &f, double a, double b) {
		using T = glm::dvec3;

		if (a >= b)
			return T(.0);

		glm::dvec3 t;
		std::array<glm::dvec3, N> Tk;
		Tk.fill(glm::dvec3(.0));

		Tk[0] = .5 * (b - a) * (f(a) + f(b));

		for (std::uint64_t n = 1; n < N; ++n) {
			std::uint64_t k = static_cast<std::uint64_t>(1) << n;
			double h = (b - a) / static_cast<double>(k);
			t = .5 * Tk[0] + h * trapezoid(f, h, a, b, k);

			for (std::uint64_t m = 1; m <= n; ++m) {
				glm::dvec3 s = t + (t - Tk[m - 1]) / static_cast<double>((1ull << 2ull * m) - 1);
				Tk[m - 1] = t;
				t = s;
			}

			Tk[n] = t;
		}

		return static_cast<T>(Tk[N - 1]);
	}
};

}

struct refraction_ratio_fit_data {
	double a1, a2, b1, b2;
};

struct refraction_ratio_fit {
	static constexpr int N = 128;
	static constexpr int M = 64;

	unsigned char ndf_type[8];
	std::uint16_t version;
	std::uint8_t  _unused;
	std::uint32_t width;
	std::uint32_t height;
	std::uint32_t hash;

	std::array<std::array<refraction_ratio_fit_data, N>, M> data;
};

using fitting_future = std::future<std::tuple<std::string, int, int>>;

bool matlab_eval(const std::string &cmd, int x, int y, int elements, Engine *matlabEngine, refraction_ratio_fit *lut, double *rmse) {
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

void fit(fitting_future &future, Engine* &matlabEngine, refraction_ratio_fit *lut) {
	if (!future.valid())
		return;

	auto fdata = future.get();

	auto cmd = std::get<0>(fdata);
	int x = std::get<1>(fdata);
	int y = std::get<2>(fdata);

	for (int itry = 0; itry < 3; ++itry) {
		double rmse;
		if (matlab_eval(cmd, x, y, 4, matlabEngine, lut, &rmse)) {
			std::cout << "<" << x << "," << y << ">: RMSE: " << rmse << "   - Fit: " << lut->data[x][y].a1 << "*exp(" << lut->data[x][y].a2 << "*x) + " << lut->data[x][y].b1 << "*exp(" << lut->data[x][y].b2 << "*x)" << std::endl;
			if (rmse > .05) {
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

double D(double cos_theta, double roughness) {
	double a = roughness * roughness;
	double t = (a*a - 1.) * cos_theta*cos_theta + 1.;
	return a*a * glm::one_over_pi<double>() / (t*t);
}

glm::dvec3 refract(const glm::dvec3 &m, const glm::dvec3 &l, double r) {
	double cosx = glm::dot(l, m);
	double sin_critical = glm::min<double>(1.0, r);
	double cos_critical = 1. - sin_critical * sin_critical;
	if (cosx * cosx <= cos_critical * cos_critical)
		return glm::dvec3{ .0 };

	double t = 1 / r;
	double c = glm::dot(l, m);
	return -t*l + (t*c - sqrt(1 - t*t*(1 - c*c)))*m;
}

glm::dvec3 refract_clamp(const glm::dvec3 &m, const glm::dvec3 &l, double r) {
	glm::dvec3 o = refract(m, l, r);

	glm::dvec3 v{ 0, 0, 0 };
	auto len = glm::length(o);
	if (len > 0 && o.z < 0)
		v = o / len;

	return v;
}

int main() {
	constexpr double Rmin = .2;
	constexpr double Rmax = 3.2;
	auto *lut = new refraction_ratio_fit;

	/*
	 * Usage example
	 *
	{
		std::ifstream ifs("microfacet_ggx_refraction_fit.bin", std::ios::binary);
		ifs.read(reinterpret_cast<char*>(lut), sizeof(*lut));
		ifs.close();

		double ior_ratio = .55;
		double roughness = .2;
		double theta = glm::half_pi<double>() * .165;
		double cos_theta = glm::cos(theta);

		int x = glm::round(glm::clamp((ior_ratio - Rmin) / (Rmax - Rmin), .0, 1.) * refraction_ratio_fit::N);
		int y = glm::round(roughness * refraction_ratio_fit::N);

		std::cout << "x: " << x << " y: " << y << std::endl;

		auto exp2_fit = lut->data[x][y];
		double Vx = exp2_fit.a1 * glm::exp(exp2_fit.a2 * cos_theta) + exp2_fit.b1 * glm::exp(exp2_fit.b2 * cos_theta);
		std::cout << Vx << std::endl;
		Vx = -glm::clamp(Vx, -1., 1.);
		std::cout << Vx << std::endl;

		glm::dvec3 v = glm::dvec3{ glm::sin(theta), 0, cos_theta };
		glm::dvec3 n = glm::dvec3{ 0, 0, 1 };
		glm::dvec3 w = glm::normalize(glm::cross(n,glm::cross(n,v)));

		std::cout << "V: <" << v.x << ", " << v.y << ", " << v.z << ">" << std::endl;
		std::cout << "N: <" << n.x << ", " << n.y << ", " << n.z << ">" << std::endl;
		std::cout << "W: <" << w.x << ", " << w.y << ", " << w.z << ">" << std::endl;

		glm::dvec3 t = Vx * w - glm::sqrt(1 - Vx*Vx) * n;
		std::cout << "T1: <" << t.x << ", " << t.y << ", " << t.z << ">" << std::endl;
	}*/

	void * vpDcom = NULL;
	int ret = 0;
	Engine* matlabEngine = engOpenSingleUse(0, vpDcom, &ret);

	fitting_future futures[2];

	int it = 0;
	constexpr double cos_theta_step = 1.0 / 100.0;
	for (int x = 0; x<refraction_ratio_fit::N; ++x) {
		for (int y = 0; y<refraction_ratio_fit::M; ++y) {
			double ior_ratio = static_cast<double>(x) / static_cast<double>(refraction_ratio_fit::N - 1) * (Rmax - Rmin) + Rmin;
			double roughness = static_cast<double>(y) / static_cast<double>(refraction_ratio_fit::M - 1);
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

					auto f = [&](double x) {
						auto g = [&](double theta_v, double theta, double phi) {
							glm::dvec3 m = { glm::sin(theta) * glm::cos(phi), glm::sin(theta) * glm::sin(phi), glm::cos(theta) };
							glm::dvec3 l = { glm::sin(theta_v), 0, glm::cos(theta_v) };

							if (!V(m, l))
								return glm::dvec3{ .0 };

							return refract_clamp(m, l, ior_ratio);
						};

						auto intg = StE::romberg_integration<10>::integrate(std::bind(g, theta, x, std::placeholders::_1), 0, glm::two_pi<double>());
						auto c = .5 * glm::sin(2 * x) * D(glm::cos(x), roughness);

						return c * intg;
					};

					double s = 0.;
					double t = glm::half_pi<double>();

					glm::dvec3 numerical_integration = StE::romberg_integration<10>::integrate(std::bind(f, std::placeholders::_1), s, t);

					auto intgv_len = glm::length(numerical_integration);
					auto intgv = intgv_len > 0 ? numerical_integration / intgv_len : glm::dvec3(0);

					double res = intgv.x;
					if (std::isnan(res)) {
						std::cout << "!! nan for " << omega<<  "," << roughness << " !!" << std::endl;
						res = .0;
					}

					strx += std::to_string(cos_theta);
					stry += std::to_string(res);
				}

				std::string cmd = "x=[" + strx + "];\ny=[" + stry + R"(];
						w = ones(length(x),1); w(1) = 10; w(length(x))=100;

						[xData, yData, weights] = prepareCurveData( x, y, w );

						ft = fittype( 'a1*exp(a2*x) + b1*exp(b2*x)', 'independent', 'x', 'dependent', 'y' );
						opts = fitoptions( 'Method', 'NonlinearLeastSquares' );
						opts.Display = 'Off';
						opts.MaxFunEvals = 10000;
						opts.MaxIter = 10000;
						opts.StartPoint = [1 1 1 1];
						opts.Weights = weights;

						[sf, sg] = fit(xData, yData, ft, opts);

						p = coeffvalues(sf);
						rmse = sg.rmse;
					)";

				return std::make_tuple(cmd, x, y);
			});

			++it;
			fit(futures[it % 2], matlabEngine, lut);

			std::cout << "iteration: " << std::to_string(it) << " (" << std::to_string(100.0*static_cast<double>(it) / static_cast<double>(refraction_ratio_fit::N*refraction_ratio_fit::N)) << "%)" << std::endl;
		}
	}

	++it;
	fit(futures[it % 2], matlabEngine, lut);

	engClose(matlabEngine);

	memcpy(lut->ndf_type, "GGX RFRC", 8);
	lut->ndf_type[0] = 'G'; lut->ndf_type[1] = 'G'; lut->ndf_type[2] = 'X';
	lut->width = refraction_ratio_fit::N;
	lut->height = refraction_ratio_fit::M;
	lut->version = 5;

	boost::crc_32_type crc_computer;
	crc_computer.process_bytes(reinterpret_cast<const std::uint8_t*>(&lut->data), sizeof(lut->data));
	lut->hash = crc_computer.checksum();

	std::ofstream ofs("microfacet_ggx_refraction_fit.bin", std::ios::binary);
	ofs.write(reinterpret_cast<const char*>(lut), sizeof(*lut));
	ofs.close();

	delete lut;

	return 0;
}
