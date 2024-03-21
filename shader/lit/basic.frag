#version 460 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normalDir;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

layout(location = 1) uniform mat4 model;
layout(location = 2) uniform mat4 view;
layout(location = 3) uniform mat4 projection;
layout(location = 4) uniform vec3 viewPos;

layout(location = 1000) uniform Light lights[4];

void main() {
    vec3 baseColor = vec3(0.84, 0.08, 0.99);
    vec3 ambient = 0.2 * baseColor;
    vec3 color = vec3(0.0);
    vec3 specular = vec3(0.0);
    for(int i = 0; i < 1; i++) {
        Light light = lights[i];
        // compute diffuse
        vec3 norm = normalize(normalDir);
        vec3 lightDir = normalize(light.position - fragPos);
        float diff = max(dot(lightDir, norm), 0.0);
        vec3 diffuse = vec3(diff * light.intensity);

        // compute specular
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 reflectDir = reflect(lightDir, norm);
        float spec = pow(max(dot(reflectDir, viewDir), 0.0), 32);
        specular = vec3(spec * light.intensity);

        color += (diffuse + specular) * light.color;
    }
    vec3 result = clamp(color * baseColor + ambient, 0.0, 1.0);
    FragColor = vec4(result, 1.0);

    FragColor = vec4(normalDir, 1.0);
    // FragColor = vec4(specular, 1.0);
}
