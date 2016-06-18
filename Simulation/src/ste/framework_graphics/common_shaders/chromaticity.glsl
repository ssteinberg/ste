
mat3 conversion_matrix_rgb_ro_xyz = mat3(
	0.412453, 0.212671, 0.019334,
	0.357580, 0.715160, 0.119193,
	0.180423, 0.072169, 0.950227
);

mat3 conversion_matrix_xyz_ro_rgb = mat3(
	+3.240479, -0.969256, +0.055648,
	-1.537150, +1.875991, -0.204043,
	-0.498535, +0.041556, +1.057311
);

vec3 RGBtoXYZ(vec3 rgb) {
	return conversion_matrix_rgb_ro_xyz * rgb;
}

vec3 XYZtoxyY(vec3 XYZ) {
	float XYZtotal = XYZ.x + XYZ.y + XYZ.z;
	return vec3(XYZ.xy / XYZtotal, XYZ.y);
}

vec3 XYZtoRGB(vec3 xyz) {
	return conversion_matrix_xyz_ro_rgb * xyz;
}

vec3 xyYtoXYZ(vec3 xyY) {
	vec3 XYZ;
	float Y_y = xyY.z / xyY.y;
	XYZ.x = Y_y * xyY.x;
	XYZ.z = Y_y * (1 - xyY.x - xyY.y);
	XYZ.y = xyY.z;
	return XYZ;
}

vec3 luminance_saturate(vec3 c) {
	return c / max(c.x, max(c.y, c.z));
}
