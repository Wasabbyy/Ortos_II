#version 330 core

layout(location = 0) in vec3 aPos; // Vertex position input

void main() {
    gl_Position = vec4(aPos, 1.0); // Transform vertex position to clip space
}