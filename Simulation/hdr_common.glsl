
const int bins = 64;
const float fbins = float(bins);
const float epsilon = .00001f;


mat3 XYZtoRGB = transpose(mat3(	 3.2404542, -1.5371385, -0.4985314,
								-0.9692660,  1.8760108,  0.0415560,
								 0.0556434, -0.2040259,  1.0572252));

mat3 RGBtoXYZ = transpose(mat3(	 0.4124564,  0.3575761,  0.1804375,
								 0.2126729,  0.7151522,  0.0721750,
								 0.0193339,  0.1191920,  0.9503041));

float hdr_bin(float max_lum, float min_lum, float l) {
	float range = max_lum - min_lum;
	float bin_size = range / fbins;
	float r = (l - min_lum) / bin_size;
	return clamp(r, 0.f, fbins - .000001f);
}

float dL_range() {
	return log(1.f + epsilon) - log(epsilon);
}

float hdr_lum(float l) {
	return log(l + epsilon) - log(epsilon);
}

float tonemap(float l) {
	return l;//exp(dL_range() * l + log(.000001f));
}
