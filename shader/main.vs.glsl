#version 150 core
#extension GL_ARB_explicit_attrib_location: enable

// input vertex data
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

// output data
out vec4 color;
out vec2 texture_coordinates;

layout(std140) uniform Materials {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
} material;

layout(std140) uniform Transformations {
	mat4 ModelViewMatrix;
	mat4 ProjectionMatrix;
	mat4 NormalMatrix;
} matrices;

// main entry point for the vertex shader
void main() {
  gl_Position = matrices.ProjectionMatrix * matrices.ModelViewMatrix * vec4(position, 1.0);
  color = material.ambient;
  texture_coordinates = uv;
}
