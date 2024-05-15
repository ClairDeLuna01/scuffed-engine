#version 460 core

layout(location = 0) in vec3 position;

layout(location = 5) uniform mat4 lightSpaceMatrix;

void main() {
    gl_Position = lightSpaceMatrix * vec4(position, 1.0);
}
