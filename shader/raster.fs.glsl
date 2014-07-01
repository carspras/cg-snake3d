#version 150 core
#extension GL_ARB_explicit_attrib_location: enable

uniform vec4 color;

// output to the (default) FBO in color buffer 0
layout (location = 0) out vec4 outColor;

// main entry point for the fragment shader
void main() {
  outColor = color;
}
