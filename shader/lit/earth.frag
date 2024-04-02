#version 460 core

in vec2 uv;
in vec3 normalDir;
in vec3 fragPos;

out vec4 FragColor;
layout(location = 4) uniform vec3 earthPos;
layout(location = 5) uniform vec3 sunPos;

layout(location = 500) uniform sampler2D TextureDay;
layout(location = 501) uniform sampler2D TextureNight;

void main() {
    vec3 sunDir = normalize(sunPos - earthPos);
    float sunIntensity = max(dot(normalDir, sunDir), 0.0);
    vec4 dayColor = texture(TextureDay, uv);
    vec4 nightColor = texture(TextureNight, uv);
    FragColor = mix(nightColor, dayColor, sunIntensity);

    // FragColor = vec4(uv, 1.0, 1.0);
}