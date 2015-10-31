
const float pi = 3.1415926535897932384626433832795;
const float pi_2 = pi * .5f;

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
