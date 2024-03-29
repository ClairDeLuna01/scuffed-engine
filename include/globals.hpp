#pragma once
#include "typedef.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace glm;

namespace EngineGlobals
{
    extern glm::ivec2 windowSize;

    extern mat4 projectionMatrix;

    extern f32 deltaTime;
    extern f32 lastFrame;

    extern ivec2 clickPos;

    extern GLFWwindow *window;

    extern GameObjectPtr sceneRoot;

    extern CameraPtr camera;

    mat4 getViewMatrix();
};