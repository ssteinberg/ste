
#include <common.glsl>

mat3 conversion_matrix_rgb_to_xyz = mat3(
	vec3(0.412453f, 0.357580f, 0.180423f),
	vec3(0.212671f, 0.715160f, 0.072169f),
	vec3(0.019334f, 0.119193f, 0.950227f)
);

mat3 conversion_matrix_xyz_to_rgb = mat3(
	vec3( 3.24048114,  -1.53715146, -0.498536319),
	vec3(-0.969254911,  1.87598991,  0.0415559262),
	vec3( 0.0556466356,-0.204041332, 1.05731094)
);

vec3 RGBtoXYZ(vec3 rgb) {
	return rgb * conversion_matrix_rgb_to_xyz;
}

vec3 XYZtoxyY(vec3 XYZ) {
	float XYZtotal = XYZ.x + XYZ.y + XYZ.z;
	return vec3(XYZ.xy / XYZtotal, XYZ.y);
}

vec3 XYZtoRGB(vec3 xyz) {
	return xyz * conversion_matrix_xyz_to_rgb;
}

vec3 xyYtoXYZ(vec3 xyY) {
	vec3 XYZ;
	float Y_y = xyY.z / xyY.y;
	XYZ.x = Y_y * xyY.x;
	XYZ.z = Y_y * (1 - xyY.x - xyY.y);
	XYZ.y = xyY.z;
	return XYZ;
}
