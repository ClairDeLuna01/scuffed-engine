#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "typedef.hpp"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include <reactphysics3d/reactphysics3d.h>
namespace rp3d = reactphysics3d;

using namespace glm;

#include <GL/glew.h>

// HEADER-ONLY
// Collection of utility functions that doesn't fit in other files

#define GENERATE_ENUM(name, ...)                                                                                       \
    enum name                                                                                                          \
    {                                                                                                                  \
        __VA_ARGS__                                                                                                    \
    };                                                                                                                 \
    std::unordered_map<std::string, name> name##S2N = {FOR_EACH(MAP_ARG1, __VA_ARGS__)};                               \
    std::unordered_map<name, std::string> name##N2S = {FOR_EACH(MAP_ARG2, __VA_ARGS__)}

#define PARENS ()

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...) macro(a1) __VA_OPT__(FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define MAP_ARG1(x) {#x, x},

#define MAP_ARG2(x) {x, #x},

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

// reactphysics Vector3 to glm vec3
inline vec3 toVec3(const rp3d::Vector3 &v)
{
    return vec3(v.x, v.y, v.z);
}

// reactphysics Quaternion to glm quat
inline quat toQuat(const rp3d::Quaternion &q)
{
    return quat(q.w, q.x, q.y, q.z);
}

// glm vec3 to reactphysics Vector3
inline rp3d::Vector3 toVec3(const vec3 &v)
{
    return rp3d::Vector3(v.x, v.y, v.z);
}

// glm quat to reactphysics Quaternion
inline rp3d::Quaternion toQuat(const quat &q)
{
    return rp3d::Quaternion(q.x, q.y, q.z, q.w);
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
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
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

inline std::ostream &operator<<(std::ostream &os, const quat &q)
{
    os << "(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
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

} // namespace glm

enum UNIFORM_LOCATIONS : GLint
{
    MODEL_MATRIX = 1,
    VIEW_MATRIX = 2,
    PROJECTION_MATRIX = 3,
    VIEW_POS = 4,
    LIGHT_SPACE_MATRIX = 5,
    SCREEN_RESOLUTION = 6,
    FOCUS_DISTANCE = 7,
    FOCUS_POSITION = 8,

    TEXTURE0 = 500,

    ENVIRONMENT_MAP = 749,
    FRAMEBUFFER = 750,
};

inline vec3 rgb(u8 r, u8 g, u8 b)
{
    return vec3(r / 255.0f, g / 255.0f, b / 255.0f);
}

inline vec3 rgb(vec3 rgbColor)
{
    return vec3(rgbColor.r / 255.0f, rgbColor.g / 255.0f, rgbColor.b / 255.0f);
}

template <typename T, std::enable_if_t<std::is_arithmetic<T>::value, bool> = true> T rand(T low, T high)
{
    return low + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX / (high - low)));
}

inline vec3 parseVec3(char *str)
{
    float x, y, z;
    int n = sscanf(str, "%f %f %f", &x, &y, &z);
    if (n == 1)
    {
        return vec3(x);
    }
    else if (n == 2)
    {
        return vec3(x, y, 0.0f);
    }
    else if (n == 3)
    {
        return vec3(x, y, z);
    }
    return vec3(0.0f);
}

inline vec3 parseColor(char *str)
{
    if (str[0] == '#') // HTML color
    {
        u32 color = std::stoul(str + 1, nullptr, 16);
        return vec3((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF) / 255.0f;
    }
    else if (str[0] == 'r' && str[1] == 'g' && str[2] == 'b') // rgb color
    {
        return parseVec3(str + 4) / 255.0f;
    }
    else // vec3 color
    {
        return parseVec3(str);
    }
}

inline vec3 normalize0(vec3 v)
{
    if (length2(v) == 0.0f)
    {
        return vec3(0.0f);
    }
    return normalize(v);
}

inline vec3 clampLength(vec3 v, f32 min, f32 max)
{
    f32 len = length(v);
    if (len < min)
    {
        return normalize(v) * min;
    }
    else if (len > max)
    {
        return normalize(v) * max;
    }
    return v;
}