#version 460 core

out vec4 FragColor;

in vec2 uv;
in vec3 fragPos;
in vec3 normalDir;

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

layout(location = 1) uniform mat4 model;
layout(location = 2) uniform mat4 view;
layout(location = 3) uniform mat4 projection;
layout(location = 4) uniform vec3 viewPos;
layout(location = 6) uniform vec2 resolution;
layout(location = 750) uniform sampler2D frameBuffer;

void main() {
    // vec3 white = vec3(1.0);
    // vec3 black = vec3(0.0);
    // // vec3 baseColor = vec3(
    // //     mix(mix(yellow, black, step(0.25, mod(uv.x, 0.5))), 
    // //     mix(black, yellow, step(0.25, mod(uv.x, 0.5))), 
    // //     step(0.5, uv.y)));

    // vec3 viewDir = normalize(viewPos - fragPos);
    // vec3 reflectDir = reflect(viewDir, normalDir);

    // vec3 baseColor = vec3(0.08, 0.25, 0.02);

    // vec3 ambient = 0.2 * baseColor;
    // vec3 color = vec3(0.0);
    // vec3 specular = vec3(0.0);
    // for(int i = 0; i < numLights; i++) {
    //     Light light = lights[i];
    //     // compute diffuse
    //     vec3 norm = normalize(normalDir);
    //     vec3 lightDir = normalize(light.position - fragPos);
    //     float diff = max(dot(lightDir, norm), 0.0);
    //     vec3 diffuse = vec3(diff * light.intensity);

    //     // compute specular
    //     vec3 reflectDir = reflect(-lightDir, norm);
    //     float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    //     specular = vec3(spec * light.intensity);

    //     color += (diffuse) * light.color;
    // }

    // // compute directional light
    // float diff = max(dot(normalDir, -dirLight.direction), 0.0);
    // vec3 diffuse = vec3(diff * dirLight.intensity) * dirLight.color;
    // color += diffuse;

    // vec3 result = clamp(color * baseColor + ambient, 0.0, 1.0);

    // // result = vec3(numLights);

    // FragColor = vec4(result, 1.0);

    vec2 screenPos = gl_FragCoord.xy / resolution;
    // float noise = texture(noiseTexture, uv).x;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(viewDir, normalDir);
    float diffractionFact = smoothstep(0.1, 2.0, (dot(viewDir, normalize(reflectDir)) + 1.0) / 2.0) * 64.0;
    vec2 diffractionDir = normalize(reflectDir.xy);

    vec2 diffractionOffset = diffractionDir * diffractionFact * 0.01;

    vec2 diffractionUV = screenPos + diffractionOffset;
    vec4 diffractionColor = texture(frameBuffer, diffractionUV);
    FragColor = diffractionColor;
    // FragColor = vec4(vec3(diffractionFact), 1.0);
    // FragColor.rgb += specular;
    // vec3 debritColor = vec3(0.5, 0.5, 0.5);
    // float hasDebrit = smoothstep(0.75, 0.85, noise);
    // FragColor = mix(FragColor, vec4(debritColor, 1.0), hasDebrit);

    // FragColor = vec4(vec3((dot(viewDir, reflectDir) + 1.0) / 2.0), 1.0);

    // FragColor = vec4(normalDir, 1.0);
    // FragColor = vec4(specular, 1.0);
}
