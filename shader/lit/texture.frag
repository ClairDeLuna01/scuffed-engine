#version 460 core

#include "common.glsl"

in vec2 uv;
in vec3 normalDir;
in vec3 fragPos;
in vec4 prevFragPos;
in vec4 glFragPos;

out vec4 FragColor;

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

    // compute directional light
    float diff = max(dot(normalDir, -dirLight.direction), 0.0);
    vec3 diffuse = vec3(diff * dirLight.intensity) * dirLight.color;
    color += diffuse;

    vec3 result = clamp(color * texColor + ambient, 0.0, 1.0);

    FragColor = vec4(result, 1.0);

    vec2 fragCoord = gl_FragCoord.xy;
    int fragPosition = int(fragCoord.x) + int(fragCoord.y) * int(resolution.x);

    vec2 a = (glFragPos.xy / glFragPos.w) * 0.5 + 0.5;
    vec2 b = (prevFragPos.xy / prevFragPos.w) * 0.5 + 0.5;

    vec2 velocity = a - b;

    velocities[fragPosition] = velocity;
}