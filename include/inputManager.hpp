#pragma once
#include "typedef.hpp"
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef std::function<void(GLFWwindow *, u32, u32, u32, u32)> keycallback_t;
typedef std::function<void(GLFWwindow *, f64, f64)> cursorcallback_t;
typedef std::function<void(GLFWwindow *, f64, f64)> scrollcallback_t;
typedef std::function<void(GLFWwindow *, u32, u32, u32)> mousebuttoncallback_t;
typedef std::function<void(GLFWwindow *, f32)> stepcallback_t;
typedef std::function<void(GLFWwindow *, i32, i32)> windowSizeCallback_t;

class InputManager
{
  private:
    static std::vector<keycallback_t> keyCallbacks;                 // key, scancode, action, mods
    static std::vector<cursorcallback_t> cursorCallbacks;           // xpos, ypos
    static std::vector<scrollcallback_t> scrollCallbacks;           // xoffset, yoffset
    static std::vector<mousebuttoncallback_t> mouseButtonCallbacks; // button, action, mods
    static std::vector<stepcallback_t> stepCallbacks;               // deltaTime
    static std::vector<windowSizeCallback_t> windowSizeCallbacks;   // width, height

  public:
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void cursorCallback(GLFWwindow *window, f64 xpos, f64 ypos);
    static void scrollCallback(GLFWwindow *window, f64 xoffset, f64 yoffset);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void stepCallback(GLFWwindow *window, f32 deltaTime);
    static void windowSizeCallback(GLFWwindow *window, i32 width, i32 height);

    static void addKeyCallback(keycallback_t callback);
    static void addCursorCallback(cursorcallback_t callback);
    static void addScrollCallback(scrollcallback_t callback);
    static void addMouseButtonCallback(mousebuttoncallback_t callback);
    static void addStepCallback(stepcallback_t callback);
    static void addWindowSizeCallback(windowSizeCallback_t callback);
};