layout(location = 4) uniform vec3 viewPos;
layout(location = 7) uniform float time;

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