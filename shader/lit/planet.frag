#version 460 core

in vec2 uv;
in vec3 normalDir;
in vec3 fragPos;

out vec4 FragColor;
layout(location = 4) uniform vec3 planetPos;
layout(location = 5) uniform vec3 sunPos;

layout(location = 500) uniform sampler2D Texture;

void main() {
    vec3 sunDir = normalize(sunPos - planetPos);
    float sunIntensity = max(dot(normalDir, sunDir), 0.0);
    vec3 sunColor = vec3(1.0, 1.0, 1.0);
    vec3 ambientColor = vec3(0.1, 0.1, 0.1);
    vec3 diffuseColor = sunIntensity * sunColor;
    vec3 result = clamp((ambientColor + diffuseColor) * texture(Texture, uv).rgb, 0.0, 1.0);
    FragColor = vec4(result, 1.0);

    // FragColor = vec4(uv, 1.0, 1.0);
}