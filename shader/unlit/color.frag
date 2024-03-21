#version 460 core

in vec3 normalDir;
in vec3 fragPos;

out vec4 FragColor;
layout(location = 0) uniform vec4 color;
layout(location = 4) uniform vec3 viewPos;

void main() {
    vec3 norm = normalize(normalDir);
    vec3 viewDir = normalize(viewPos - fragPos);
    float diff = max(dot(norm, viewDir), 0.0);
    vec3 diffuse = diff * color.rgb;
    FragColor = vec4(norm, 1.0);
}
