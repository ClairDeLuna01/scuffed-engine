#include "globals.hpp"

ivec2 EngineGlobals::windowSize = ivec2(800, 600);

mat4 EngineGlobals::projectionMatrix = mat4(1.0f);

f32 EngineGlobals::deltaTime = 0.0f;
f32 EngineGlobals::lastFrame = 0.0f;

ivec2 EngineGlobals::clickPos = ivec2(-1.0f, -1.0f);

GLFWwindow *EngineGlobals::window = nullptr;

GameObjectPtr EngineGlobals::sceneRoot = createGameObject();

CameraPtr EngineGlobals::camera = createCamera();

MeshPtr EngineGlobals::skybox = nullptr;

UI *EngineGlobals::ui;