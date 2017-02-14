
#include <stdafx.hpp>
#include <XYZ.hpp>
#include <kelvin.hpp>

using namespace StE::Graphics;

kelvin XYZ::toKelvin() const {
	/*
	 *	Thanks to Bruce Lindbloom and A. R. Robertson:
	 *	http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_T.html
	 *
	 *	Copyright © 2003 Bruce Justin Lindbloom.
	*/
	typedef struct UVT {
		double  u;
		double  v;
		double  t;
	} UVT;

	double rt[31] = {       /* reciprocal temperature (K) */
		std::numeric_limits<double>::min(),
		10.0e-6,  20.0e-6,  30.0e-6,  40.0e-6,  50.0e-6,
		60.0e-6,  70.0e-6,  80.0e-6,  90.0e-6, 100.0e-6, 125.0e-6,
		150.0e-6, 175.0e-6, 200.0e-6, 225.0e-6, 250.0e-6, 275.0e-6,
		300.0e-6, 325.0e-6, 350.0e-6, 375.0e-6, 400.0e-6, 425.0e-6,
		450.0e-6, 475.0e-6, 500.0e-6, 525.0e-6, 550.0e-6, 575.0e-6,
		600.0e-6
	};

	UVT uvt[31] = {
		{0.18006, 0.26352, -0.24341},
		{0.18066, 0.26589, -0.25479},
		{0.18133, 0.26846, -0.26876},
		{0.18208, 0.27119, -0.28539},
		{0.18293, 0.27407, -0.30470},
		{0.18388, 0.27709, -0.32675},
		{0.18494, 0.28021, -0.35156},
		{0.18611, 0.28342, -0.37915},
		{0.18740, 0.28668, -0.40955},
		{0.18880, 0.28997, -0.44278},
		{0.19032, 0.29326, -0.47888},
		{0.19462, 0.30141, -0.58204},
		{0.19962, 0.30921, -0.70471},
		{0.20525, 0.31647, -0.84901},
		{0.21142, 0.32312, -1.0182},
		{0.21807, 0.32909, -1.2168},
		{0.22511, 0.33439, -1.4512},
		{0.23247, 0.33904, -1.7298},
		{0.24010, 0.34308, -2.0637},
		{0.24792, 0.34655, -2.4681},	/* Note: 0.24792 is a corrected value for the error found in W&S as 0.24702 */
		{0.25591, 0.34951, -2.9641},
		{0.26400, 0.35200, -3.5814},
		{0.27218, 0.35407, -4.3633},
		{0.28039, 0.35577, -5.3762},
		{0.28863, 0.35714, -6.7262},
		{0.29685, 0.35823, -8.5955},
		{0.30505, 0.35907, -11.324},
		{0.31320, 0.35968, -15.628},
		{0.32129, 0.36011, -23.325},
		{0.32931, 0.36038, -40.770},
		{0.33724, 0.36051, -116.45}
	};

	kelvin ret;
	double xyz[3];
	double us, vs, p, di, dm;
	int i;

	xyz[0] = X();
	xyz[1] = Y();
	xyz[2] = Z();

	if ((xyz[0] < 1.0e-20) && (xyz[1] < 1.0e-20) && (xyz[2] < 1.0e-20)) {
		ret.K() = .0f;
	}
	else {
		us = (4.0 * xyz[0]) / (xyz[0] + 15.0 * xyz[1] + 3.0 * xyz[2]);
		vs = (6.0 * xyz[1]) / (xyz[0] + 15.0 * xyz[1] + 3.0 * xyz[2]);
		dm = 0.0;
		for (i = 0; i < 31; i++) {
			di = (vs - uvt[i].v) - uvt[i].t * (us - uvt[i].u);
			if ((i > 0) && (((di < 0.0) && (dm >= 0.0)) || ((di >= 0.0) && (dm < 0.0))))
				break;  /* found lines bounding (us, vs) : i-1 and i */
			dm = di;
		}
		if (i == 31)
			ret.K() = .0f;
		else {
			di = di / glm::sqrt(1.0 + uvt[i    ].t * uvt[i    ].t);
			dm = dm / glm::sqrt(1.0 + uvt[i - 1].t * uvt[i - 1].t);
			p = dm / (dm - di);     /* p = interpolation parameter, 0.0 : i-1, 1.0 : i */
			p = 1.0 / (glm::mix(rt[i - 1], rt[i], p));
			ret.K() = p;
		}
	}

	return ret;
}
