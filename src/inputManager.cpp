#include "inputManager.hpp"

std::vector<keycallback_t> InputManager::keyCallbacks;
std::vector<cursorcallback_t> InputManager::cursorCallbacks;
std::vector<scrollcallback_t> InputManager::scrollCallbacks;
std::vector<stepcallback_t> InputManager::stepCallbacks;
std::vector<mousebuttoncallback_t> InputManager::mouseButtonCallbacks;
std::vector<windowSizeCallback_t> InputManager::windowSizeCallbacks;

void InputManager::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    for (auto callback : keyCallbacks)
    {
        callback(window, key, scancode, action, mods);
    }
}

void InputManager::cursorCallback(GLFWwindow *window, f64 xpos, f64 ypos)
{
    for (auto callback : cursorCallbacks)
    {
        callback(window, xpos, ypos);
    }
}

void InputManager::scrollCallback(GLFWwindow *window, f64 xoffset, f64 yoffset)
{
    for (auto callback : scrollCallbacks)
    {
        callback(window, xoffset, yoffset);
    }
}

void InputManager::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    for (auto callback : mouseButtonCallbacks)
    {
        callback(window, button, action, mods);
    }
}

void InputManager::stepCallback(GLFWwindow *window, f32 deltaTime)
{
    for (auto callback : stepCallbacks)
    {
        callback(window, deltaTime);
    }
}

void InputManager::windowSizeCallback(GLFWwindow *window, i32 width, i32 height)
{
    for (auto callback : windowSizeCallbacks)
    {
        callback(window, width, height);
    }
}

void InputManager::addKeyCallback(keycallback_t callback)
{
    keyCallbacks.push_back(callback);
}

void InputManager::addCursorCallback(cursorcallback_t callback)
{
    cursorCallbacks.push_back(callback);
}

void InputManager::addScrollCallback(scrollcallback_t callback)
{
    scrollCallbacks.push_back(callback);
}

void InputManager::addMouseButtonCallback(mousebuttoncallback_t callback)
{
    mouseButtonCallbacks.push_back(callback);
}

void InputManager::addStepCallback(stepcallback_t callback)
{
    stepCallbacks.push_back(callback);
}

void InputManager::addWindowSizeCallback(windowSizeCallback_t callback)
{
    windowSizeCallbacks.push_back(callback);
}