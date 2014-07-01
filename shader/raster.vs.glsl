#version 150 core
#extension GL_ARB_explicit_attrib_location: enable

// input vertex data
layout(location = 0) in vec3 position;

uniform mat4 MVP;

layout(std140) uniform Materials {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
} material;


// main entry point for the vertex shader
void main() {
  gl_Position = MVP * vec4(position, 1.0);
}
