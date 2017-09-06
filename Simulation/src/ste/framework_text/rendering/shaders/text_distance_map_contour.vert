
#type vert
#version 450

layout(location = 0) in uvec4 data;

layout(location = 0) out vs_out {
	uvec2 position;
	vec3 color;
	vec3 stroke_color;
	uint drawId;
	float weight;
	float stroke_width;
	float size;
} vout;

const float size_scale = 32.f;
const float weight_scale = .25f;
const float stroke_width_scale = .25f;

void main() {
	uint position_pack = data.x;
	uint glyph_index = data.y >> 16;
	float size = float(data.y & 0xFFFF) / size_scale;

	vec4 pack_a = unpackUnorm4x8(data.z);
	vec4 pack_b = unpackUnorm4x8(data.w);

	vec3 col = pack_a.rgb;
	vec3 stroke_col = pack_b.rgb;
	float weight = (pack_a.w - .5f) / weight_scale;
	float stroke_width = pack_b.w / stroke_width_scale;

	vout.position = uvec2(position_pack & 0xFFFF, position_pack >> 16);
	vout.drawId = glyph_index;
	vout.size = size;
	vout.weight = weight;
	vout.color = col;
	vout.stroke_color = stroke_col;
	vout.stroke_width = stroke_width;
}
