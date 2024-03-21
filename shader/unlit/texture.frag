#version 460 core

in vec2 uv;
in vec3 fragPos;

out vec4 FragColor;

layout(location = 500) uniform sampler2D Texture;

void main() {
    vec4 texColor = texture(Texture, uv);
    FragColor = texColor;

    // FragColor = vec4(uv, 1.0, 1.0);
}