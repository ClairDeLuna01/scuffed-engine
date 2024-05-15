#version 460 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 fragPosWorld;
in vec3 normalDir;
in float depth;

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
layout(location = 7) uniform float focusDistance;
layout(location = 8) uniform vec3 focusPos;

void main() {
    float radius = 0.15;
    // check if fragment is within radius of the center of the screen
    vec2 fragScreenPos = gl_FragCoord.xy / resolution;
    float aspectRatio = resolution.x / resolution.y;
    fragScreenPos.x *= aspectRatio;
    vec2 center = vec2(0.5 * aspectRatio, 0.5);
    float dist = distance(fragScreenPos, center);

    if(dist < radius) {
        // if fragment is in front of the focus distance, discard it
        float far = 1000.0;
        float near = 0.1;
        // float focusDepth = (2.0 * near) / (far + near - focusDistance * (far - near));
        float focusDepth = focusDistance + 2.0;

        if((depth < focusDepth) && (fragPosWorld.z > focusPos.z)) {
            discard;
        }
    }

    vec3 baseColor = vec3(0.5);
    vec3 ambient = 0.2 * baseColor;
    vec3 color = vec3(0.0);
    vec3 specular = vec3(0.0);
    for(int i = 0; i < numLights; i++) {
        Light light = lights[i];
        // compute diffuse
        vec3 norm = normalize(normalDir);
        vec3 lightDir = normalize(light.position - fragPos);
        float diff = max(dot(lightDir, norm), 0.0);
        vec3 diffuse = vec3(diff * light.intensity);

        // // compute specular
        // vec3 viewDir = normalize(viewPos - fragPos);
        // vec3 reflectDir = reflect(lightDir, norm);
        // float spec = pow(max(dot(reflectDir, viewDir), 0.0), 32);
        // specular = vec3(spec * light.intensity);

        color += (diffuse + specular) * light.color;
    }

    // compute directional light
    float diff = max(dot(normalDir, -dirLight.direction), 0.0);
    vec3 diffuse = vec3(diff * dirLight.intensity) * dirLight.color;
    color += diffuse;

    vec3 result = clamp(color * baseColor + ambient, 0.0, 1.0);

    // result = vec3(numLights);

    FragColor = vec4(result, 1.0);
    // FragColor = vec4(vec3(depth > focusDistance), 1.0);

    // FragColor = vec4(normalDir, 1.0);
    // FragColor = vec4(specular, 1.0);
}
