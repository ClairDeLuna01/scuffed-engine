#pragma once
#include <typedef.hpp>

#include <glm/glm.hpp>

using namespace glm;

namespace EngineGlobals
{
    extern glm::ivec2 windowSize;

    extern mat4 projectionMatrix;
    extern mat4 viewMatrix;

    extern f32 deltaTime;
    extern f32 lastFrame;

    extern ivec2 clickPos;
};