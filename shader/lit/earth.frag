#version 460 core

in vec2 uv;
in vec3 normalDir;
in vec3 fragPos;

out vec4 FragColor;
layout(location = 4) uniform vec3 earthPos;

struct Light {
                     // base alignment  | aligned offset
    vec3 position;   // 12 bytes        | 0
    vec3 color;      // 12 bytes        | 16
    float intensity; //  4 bytes        | 32
                     // total: 32 bytes
};

struct DirectionalLight {
                        // base alignment  | aligned offset
    vec3 direction;  // 12 bytes        | 0
    vec3 color;      // 12 bytes        | 16
    float intensity; //  4 bytes        | 32
                        // total: 32 bytes
};

layout(std140, binding = 0) uniform Lights {
                               // base alignment  | aligned offset
    Light lights[10];          // 320 bytes       | 0
    DirectionalLight dirLight; // 32 bytes        | 320
    int numLights;             //   4 bytes       | 352
                               // total: 356 bytes
};

layout(location = 500) uniform sampler2D TextureDay;
layout(location = 501) uniform sampler2D TextureNight;

void main() {
    vec4 dayColor = texture(TextureDay, uv);
    vec4 nightColor = texture(TextureNight, uv);

    float sunIntensity = 0.0;
    vec3 tint = vec3(0.0);
    for(int i = 0; i < numLights; i++) {
        Light light = lights[i];
        vec3 lightDir = normalize(light.position - fragPos);
        sunIntensity += max(dot(lightDir, normalDir), 0.0) * light.intensity;
        tint += sunIntensity * light.color;
    }

    FragColor = mix(nightColor, dayColor * vec4(tint, 1), sunIntensity);

    // FragColor = vec4(uv, 1.0, 1.0);
}