#version 460 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D glyph;
layout(location = 100) uniform vec4 textColor;
layout(location = 101) uniform vec4 backgroundColor;
layout(location = 102) uniform bool bold;
layout(location = 103) uniform bool italic;
layout(location = 104) uniform bool underline;
layout(location = 105) uniform bool strikethrough;
layout(location = 106) uniform bool outline;

layout(location = 7) uniform float time;

void main() {
    vec2 uv = TexCoords;
    if(italic) {
        uv.x += (uv.y - 0.5) * 0.16;
    }

    float sampled = texture(glyph, uv).r;
    color = textColor;

    const float defaultThickness = 0.50;
    const float boldThickness = 0.43;

    float cutoff = (bold ? boldThickness : defaultThickness);

    // const float smoothness = 0.05;

    // color.a = smoothstep(smoothness, -smoothness, dist);

    float dist = cutoff - sampled;
    // https://drewcassidy.me/2020/06/26/sdf-antialiasing/
    // vec2 ddist = vec2(dFdx(dist), dFdy(dist));
    // float pixelWidth = length(ddist);
    // color.a = clamp(0.5 - dist / pixelWidth, 0.0, 1.0);

    const float softness = 0.05;
    float a = smoothstep(cutoff - softness, cutoff + softness, sampled);
    color.a = a * textColor.a;

    // color.rgb = mix(backgroundColor.rgb, textColor.rgb, color.a);

    // outline
    if(outline) {
        const float outlineThickness = 0.025;
        const float outlineSoftness = 0.05;
        const vec3 outlineColor = vec3(0.0, 0.0, 0.0);

        float outlineDist = abs(dist) - outlineThickness;
        float outlineAlpha = smoothstep(0.0, outlineSoftness, outlineDist);
        color.rgb = mix(outlineColor, color.rgb, outlineAlpha);
    }

    // color.rgb = vec3(outlineDist);

    // color = textColor;
}