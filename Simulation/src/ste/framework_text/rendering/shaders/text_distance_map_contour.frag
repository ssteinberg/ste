
#type frag
#version 450
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

struct buffer_glyph_descriptor {
	int16_t width;
	int16_t height;
	int16_t start_y;
	int16_t start_x;
	uint64_t tex_handler;
};

out vec4 gl_FragColor;

in geo_out {
	vec4 color;
	vec4 stroke_color;
	float weight;
	float stroke_width;
	vec2 st;
	flat int drawId;
} vin;

layout(std430, binding = 0) buffer glyph_data {
	buffer_glyph_descriptor glyphs[];
};

float aastep (float threshold , float value) {
	float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
	return smoothstep(threshold-afwidth, threshold+afwidth, value);
}

void main( void ) {
	buffer_glyph_descriptor glyph = glyphs[vin.drawId];

	vec2 uv = vin.st;

	float D = textureLod(sampler2D(glyph.tex_handler), uv, 0).x;

	D -= vin.weight;

	if (vin.stroke_width > 0)
		D -= vin.stroke_width*.5f;

    float g = 1.0f - aastep(0.0, D);
	if (g==0)
		discard;
		
	vec4 c = vin.color;
	if (vin.stroke_width > 0)
		c = mix(vin.stroke_color, vin.color, clamp((- D - vin.stroke_width * .9f) / (vin.stroke_width * .2f), 0, 1));

	gl_FragColor = c * vec4(1, 1, 1, g);
}
