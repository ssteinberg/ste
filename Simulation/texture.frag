
#type frag
#version 440

in vec2 frag_texcoords;

out vec4 gl_FragColor;

uniform sampler2D tex;

void main() {
    gl_FragColor = vec4(0,0,texture(tex, frag_texcoords).r, 1);
}
