#include "globals.hpp"
#include "glm/gtc/matrix_transform.hpp"

ivec2 EngineGlobals::windowSize = ivec2(800, 600);
f32 EngineGlobals::fov = 45.0f;

mat4 EngineGlobals::projectionMatrix = mat4(1.0f);

void EngineGlobals::refreshProjectionMatrix()
{
    EngineGlobals::projectionMatrix =
        glm::perspective(radians(EngineGlobals::fov),
                         (f32)EngineGlobals::windowSize.x / (f32)EngineGlobals::windowSize.y, 0.1f, 1000.0f);
}

f32 EngineGlobals::deltaTime = 0.0f;
f32 EngineGlobals::fixedDeltaTime = 0.02f; // 50fps
f32 EngineGlobals::lastFrame = 0.0f;

ivec2 EngineGlobals::clickPos = ivec2(-1.0f, -1.0f);

GLFWwindow *EngineGlobals::window = nullptr;

CameraPtr EngineGlobals::camera = createCamera();

ScenePtr EngineGlobals::scene = nullptr;

std::vector<float> EngineGlobals::clearVelocitySSBO = std::vector<float>(windowSize.x * windowSize.y * 2, 0.0f);
u32 EngineGlobals::clearVelocitySSBOID = 0;