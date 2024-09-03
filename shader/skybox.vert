#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 1) uniform mat4 model;
layout(location = 2) uniform mat4 view;
layout(location = 3) uniform mat4 projection;
layout(location = 8) uniform mat4 prevMVP;

out vec3 fragPos;
out vec4 glFragPos;
out vec4 prevFragPos;

void main() {
    vec4 pos = projection * view * vec4(position, 1.0);
    fragPos = position;

    gl_Position = pos.xyww;
    prevFragPos = prevMVP * vec4(position, 1.0);
    glFragPos = gl_Position;
}
