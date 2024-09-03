#version 460 core
out vec4 FragColor;

#include "common.glsl"

in vec3 fragPos;
in vec4 glFragPos;
in vec4 prevFragPos;

layout(location = 500) uniform samplerCube skybox;

void main() {
    FragColor = texture(skybox, fragPos);

    vec2 fragCoord = gl_FragCoord.xy;
    int fragPosition = int(fragCoord.x) + int(fragCoord.y) * int(resolution.x);

    // vec2 a = (glFragPos.xy / glFragPos.w) * 0.5 + 0.5;
    // vec2 b = (prevFragPos.xy / prevFragPos.w) * 0.5 + 0.5;

    // vec2 velocity = a - b;

    velocities[fragPosition] = vec2(0.0);
}