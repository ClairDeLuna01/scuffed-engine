#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 1) uniform mat4 model;
layout(location = 2) uniform mat4 view;
layout(location = 3) uniform mat4 projection;
layout(location = 8) uniform mat4 prevMVP;
layout(location = 4) uniform vec3 viewPos;
layout(location = 6) uniform vec2 resolution;

layout(location = 750) uniform sampler2D fbo0;
layout(location = 751) uniform sampler2D fbo1;
layout(location = 752) uniform sampler2D fbo2;
layout(location = 753) uniform sampler2D fbo3;
layout(location = 754) uniform sampler2D fbo4;
layout(location = 755) uniform sampler2D fbo5;
layout(location = 756) uniform sampler2D fbo6;
layout(location = 757) uniform sampler2D fbo7;

out vec3 normalDir;
out vec2 uv;

void main() {
    gl_Position = vec4(position.xy, 0.0, 1.0);
    normalDir = normal;
    uv = texCoord;
}
