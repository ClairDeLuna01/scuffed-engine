#version 460 core

in vec2 uv;
in vec3 normalDir;
in vec3 fragPos;

out vec4 FragColor;
layout(location = 4) uniform vec3 planetPos;

struct Light {
                     // base alignment  | aligned offset
    vec3 position;   // 12 bytes        | 0
    vec3 color;      // 12 bytes        | 16
    float intensity; //  4 bytes        | 32
                     // total: 32 bytes
};

layout(std140, binding = 0) uniform Lights {
                           // base alignment  | aligned offset
    Light lights[10];      // 320 bytes       | 0
    int numLights;         //   4 bytes       | 320
                           // total: 336 bytes
};

layout(location = 500) uniform sampler2D Texture;

void main() {
    vec3 texColor = texture(Texture, uv).rgb;
    vec3 ambient = 0.2 * texColor;
    vec3 color = vec3(0.0);
    for(int i = 0; i < numLights; i++) {
        Light light = lights[i];
        vec3 norm = normalize(normalDir);
        vec3 lightDir = normalize(light.position - fragPos);
        float diff = max(dot(lightDir, norm), 0.0);
        vec3 diffuse = vec3(diff * light.intensity);

        color += diffuse * light.color;
    }

    vec3 result = clamp(color * texColor + ambient, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}