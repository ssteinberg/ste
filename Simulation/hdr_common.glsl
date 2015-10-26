
const int bins = 64;
const float fbins = float(bins);
const float epsilon = .00001f;
const float pi = 3.1415926535897932384626433832795;


struct hdr_bokeh_parameters {
	int lum_min, lum_max;
	float focus;
};

struct bokeh_point_descriptor {
	vec4 pos_size;
	vec4 color;
};

vec3 RGBtoXYZ(vec3 rgb) {
	vec3 xyz;
	xyz.x = 0.412453*rgb.r + 0.357580*rgb.g + 0.180423*rgb.b;
	xyz.y = 0.212671*rgb.r + 0.715160*rgb.g + 0.072169*rgb.b;
	xyz.z = 0.019334*rgb.r + 0.119193*rgb.g + 0.950227*rgb.b;
	return xyz;
}

vec3 XYZtoxyY(vec3 XYZ) {
	float XYZtotal = XYZ.x + XYZ.y + XYZ.z;
	if (XYZtotal <= 0)
		return vec3(0);
	return vec3(XYZ.xy / XYZtotal, XYZ.y);
}

vec3 XYZtoRGB(vec3 xyz) {
	vec3 rgb;
	rgb.r = 3.240479*xyz.x - 1.537150*xyz.y - 0.498535*xyz.z;
	rgb.g =-0.969256*xyz.x + 1.875991*xyz.y + 0.041556*xyz.z;
	rgb.b = 0.055648*xyz.x - 0.204043*xyz.y + 1.057311*xyz.z;
	return rgb;
}

vec3 xyYtoXYZ(vec3 xyY) {
	if (xyY.y <= 0)
		return vec3(0);
	vec3 XYZ;
	float Y_y = xyY.z / xyY.y;
	XYZ.x = Y_y * xyY.x;
	XYZ.z = Y_y * (1 - xyY.x - xyY.y);
	XYZ.y = xyY.z;
	return XYZ;
}

float hdr_bin(float max_lum, float min_lum, float l) {
	float range = max_lum - min_lum;
	float bin_size = range / fbins;
	float r = (l - min_lum) / bin_size;
	return clamp(r, 0.f, fbins - .00001f);
}

float hdr_lum(float l) {
	return log(l + epsilon) - log(epsilon);
}

float tonemap(float l) {
	return l;//exp(dL_range() * l + log(.000001f));
}
