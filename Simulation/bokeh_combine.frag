
#type frag
#version 450

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D bokeh;

void main() {
	vec4 bokeh_accum_buffer_texel = texelFetch(bokeh, ivec2(gl_FragCoord.xy), 0);

    gl_FragColor = bokeh_accum_buffer_texel;
}
