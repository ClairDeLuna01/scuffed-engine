#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <utility>

#include "typedef.hpp"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include <GL/glew.h>

// HEADER-ONLY
// Collection of utility functions that doesn't fit in other files

#define GENERATE_ENUM(name, ...)                        \
    enum name                                           \
    {                                                   \
        __VA_ARGS__                                     \
    };                                                  \
    std::unordered_map<std::string, name> name##S2N = { \
        FOR_EACH(MAP_ARG1, __VA_ARGS__)};               \
    std::unordered_map<name, std::string> name##N2S = { \
        FOR_EACH(MAP_ARG2, __VA_ARGS__)}

#define PARENS ()

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...) \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...) \
    macro(a1)                           \
        __VA_OPT__(FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define MAP_ARG1(x) \
    {#x, x},

#define MAP_ARG2(x) \
    {x, #x},

inline std::string stripPath(std::string src)
{

    size_t lastSlashIndex = 0;
    size_t size = src.length();
    for (size_t i = 0; i < size; i++)
    {
        if (src[i] == '/')
            lastSlashIndex = i;
    }

    return src.substr(lastSlashIndex + 1);
}

inline std::string getExtension(std::string src)
{

    size_t dotIndex = 0;
    bool foundDot = false;
    size_t size = src.length();
    for (size_t i = 0; i < size; i++)
    {
        if (src[i] == '.')
        {
            dotIndex = i;
            if (!foundDot)
            {
                foundDot = true;
            }
            else
            {
                std::cerr << "Path provided for shader " << src << " contains an ambiguous extension\n";
                return "";
            }
        }
    }

    return src.substr(dotIndex + 1);
}

inline u64 GetTimeMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline vec3 getEulerAngles(mat4 objMat)
{
    return eulerAngles(toQuat(objMat));
}

namespace glm
{
    inline std::ostream &operator<<(std::ostream &os, const vec3 &v)
    {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, const vec4 &v)
    {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, const mat4 &m)
    {
        os << "[";
        for (int i = 0; i < 4; i++)
        {
            os << "[";
            for (int j = 0; j < 4; j++)
            {
                os << m[j][i];
                if (j != 3)
                    os << ", ";
            }
            os << "]";
            if (i != 3)
                os << "\n";
        }
        os << "]";
        return os;
    }
}

enum UNIFORM_LOCATIONS : GLint
{
    MODEL_MATRIX = 1,
    VIEW_MATRIX = 2,
    PROJECTION_MATRIX = 3,
    VIEW_POS = 4,

    TEXTURE0 = 500,

    ENVIRONMENT_MAP = 749,

    LIGHT_POSITION = 750,
};

inline vec3 rgb(u8 r, u8 g, u8 b)
{
    return vec3(r / 255.0f, g / 255.0f, b / 255.0f);
}

inline vec3 rgb(vec3 rgbColor)
{
    return vec3(rgbColor.r / 255.0f, rgbColor.g / 255.0f, rgbColor.b / 255.0f);
}

template <typename T, std::enable_if_t<std::is_arithmetic<T>::value, bool> = true>
T rand(T low, T high)
{
    return low + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX / (high - low)));
}
