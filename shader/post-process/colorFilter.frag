#version 460 core

out vec4 FragColor;

in vec2 uv;

layout(location = 752) uniform sampler2D fbo2;

layout(location = 6) uniform vec2 resolution;

void main() {
    vec2 screenPos = gl_FragCoord.xy / resolution;
    vec3 color = texture(fbo2, screenPos).rgb;
    // color = vec3(1.0) - color;

    FragColor = vec4(color, 1.0);
    // FragColor = vec4(1.0);
}