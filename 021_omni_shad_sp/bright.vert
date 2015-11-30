#version 430 core

layout (location = 0) in vec3 vp;

uniform mat4 P, V, M;

void main () {
	gl_Position = P * V * M * vec4 (vp, 1.0);
}