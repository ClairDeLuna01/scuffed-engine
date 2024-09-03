#version 460 core

#include "common.glsl"

out vec4 FragColor;

in vec2 uv;

layout(location = 752) uniform sampler2D fbo2;

void main() {
    discard;
    vec2 screenPos = gl_FragCoord.xy / resolution;
    vec3 color = texture(fbo2, screenPos).rgb;
    // color = vec3(1.0) - color;

    vec2 fragCoord = gl_FragCoord.xy;
    int fragPosition = int(fragCoord.x) + int(fragCoord.y) * int(resolution.x);

    // FragColor = vec4(color, 1.0);
    // FragColor = vec4(1.0);

    // FragColor = vec4(clamp(color + vec3(velocities[fragPosition] * 100.0, 0.0), 0.0, 1.0), 1.0);

    vec2 vel = velocities[fragPosition];

    // move the pixel by the velocity
    vec2 offset = vel * 10.0;

    vec2 newUV = screenPos - offset;

    // FragColor = vec4(texture(fbo2, newUV).rgb, 1.0);

}