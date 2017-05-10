
#type frag
#version 450

struct buffer_glyph_descriptor {
	uint width;
	uint height;
	int start_y;
	int start_x;

	uint sampler_idx;
};

in geo_out {
	flat vec3 color;
	flat vec3 stroke_color;
	flat float weight;
	flat float stroke_width;
	flat uint drawId;
	noperspective vec2 tex_coords;
} vin;

layout(location = 0) out vec4 frag_color;

layout(std430, binding = 0) restrict readonly buffer glyph_data {
	buffer_glyph_descriptor glyphs[];
};

layout(constant_id = 0) const int glyph_texture_count = 2;
layout(binding = 1) uniform texture2D glyph_textures[glyph_texture_count];
layout(binding = 2) uniform sampler glyph_sampler;

float aastep(float threshold, float value) {
	float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
	return smoothstep(threshold-afwidth, threshold+afwidth, value);
}

void main() {
	buffer_glyph_descriptor glyph = glyphs[vin.drawId];

	vec2 uv = vin.tex_coords;

	float D = textureLod(sampler2D(glyph_textures[glyph.sampler_idx], glyph_sampler), uv, 0).x;

	D -= vin.weight;

	if (vin.stroke_width > 0)
		D -= vin.stroke_width*.5f;

	float g = 1.0f - aastep(0.0, D);
	if (g==0)
		discard;

	vec3 c = vin.color;
	if (vin.stroke_width > 0)
		c = mix(vin.stroke_color, c, clamp((- D - vin.stroke_width * .9f) / (vin.stroke_width * .2f), 0, 1));
		
	frag_color = vec4(c, g);
}
