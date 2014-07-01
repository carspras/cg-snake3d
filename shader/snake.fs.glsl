#version 150 core
#extension GL_ARB_explicit_attrib_location: enable

// input data
in vec4 color;
in vec2 texture_coordinates;

// output to the (default) FBO in color buffer 0
layout (location = 0) out vec4 outColor;

uniform sampler2D basic_texture;

// main entry point for the fragment shader
void main() {
	outColor = texture(basic_texture, texture_coordinates);
}
