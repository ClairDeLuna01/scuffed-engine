#ifndef GLUTILS_HPP
#define GLUTILS_HPP

#include <iostream>
#include <deque>
#include <string.h>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "typedef.hpp"
#include "utils.hpp"

#include "InputManager.hpp"
#include "globals.hpp"
using namespace EngineGlobals;

// #include <Utils.hpp>
inline const std::string TERMINAL_ERROR = "\e[1;31m"; //"\033[91m";
inline const std::string TERMINAL_INFO = "\033[94m";
inline const std::string TERMINAL_OK = "\033[92m";
inline const std::string TERMINAL_RESET = "\033[0m";
inline const std::string TERMINAL_TIMER = "\033[93m";
inline const std::string TERMINAL_FILENAME = "\033[95m";
inline const std::string TERMINAL_WARNING = "\e[38;5;208m";
inline const std::string TERMINAL_NOTIF = "\e[1;36m";

// #define SHOW_GL_NOTIF
// #define SHOW_GL_WARNING
#define PREVENT_GL_NOTIF_SPAM

struct GLenum_t
{
    GLenum val;
    GLenum_t(GLenum val) : val(val){};
};

struct errorHistoric
{
    GLuint id;
    u64 time;
};

std::ostream &operator<<(std::ostream &os, GLenum_t e);

void printGLerror(
    GLenum _source,
    GLenum _type,
    GLuint id,
    GLenum _severity,
    GLsizei length,
    const GLchar *message);

void GLAPIENTRY MessageCallback(GLenum _source,
                                GLenum _type,
                                GLuint id,
                                GLenum _severity,
                                GLsizei length,
                                const GLchar *message,
                                const void *userParam);

inline void resizeCallback(GLFWwindow *window, i32 width, i32 height)
{
    windowSize = ivec2(width, height);
    glViewport(0, 0, width, height);
    projectionMatrix = perspective(radians(45.0f), (f32)width / (f32)height, 0.1f, 1000.0f);
}

void glfw_error_callback(i32 error, const char *description);

void OpenGLInit();

#endif