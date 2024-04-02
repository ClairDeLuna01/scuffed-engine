#version 460 core
out vec4 FragColor;

in vec3 fragPos;

layout(location = 500) uniform samplerCube skybox;

void main() {
    FragColor = texture(skybox, fragPos);
}