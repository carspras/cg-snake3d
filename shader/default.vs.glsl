#version 150 core
#extension GL_ARB_explicit_attrib_location: enable

// input vertex data
layout(location = 0) in vec3 position;

uniform mat4 MVP;

// main entry point for the vertex shader
void main() {
  gl_Position = MVP * vec4(position, 1.0);
}
