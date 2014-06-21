#version 150 core
#extension GL_ARB_explicit_attrib_location: enable

// per-vertex attribute
layout (location = 0) in vec3 position;

// main entry point for the vertex shader
void main() {
  gl_Position = vec4(position, 1.0);
}
