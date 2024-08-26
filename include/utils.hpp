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

inline std::string upperCase(std::string src)
{
    std::string result = src;
    for (char &c : result)
    {
        c = std::toupper(c);
    }
    return result;
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
    // FOCUS_DISTANCE = 7,
    // FOCUS_POSITION = 8,
    TIME = 7,

    FONT_COLOR = 100,
    FONT_BACKGROUND_COLOR = 101,
    FONT_BOLD = 102,
    FONT_ITALIC = 103,
    FONT_UNDERLINE = 104,
    FONT_STRIKETHROUGH = 105,
    FONT_OUTLINE = 106,

    TEXTURE0 = 500,

    ENVIRONMENT_MAP = 749,
    FRAMEBUFFER0 = 750,
};

inline constexpr vec3 rgb(u8 r, u8 g, u8 b)
{
    return vec3(r / 255.0f, g / 255.0f, b / 255.0f);
}

inline constexpr vec3 rgb(vec3 rgbColor)
{
    return vec3(rgbColor.r / 255.0f, rgbColor.g / 255.0f, rgbColor.b / 255.0f);
}

template <typename T, std::enable_if_t<std::is_arithmetic<T>::value, bool> = true> T rand(T low, T high)
{
    return low + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX / (high - low)));
}

inline vec3 parseVec3(const char *str)
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

inline vec4 parseVec4(const char *str)
{
    float x, y, z, w;
    int n = sscanf(str, "%f %f %f %f", &x, &y, &z, &w);
    if (n == 1)
    {
        return vec4(x);
    }
    else if (n == 2)
    {
        return vec4(x, y, 0.0f, 0.0f);
    }
    else if (n == 3)
    {
        return vec4(x, y, z, 0.0f);
    }
    else if (n == 4)
    {
        return vec4(x, y, z, w);
    }
    return vec4(0.0f);
}

inline vec3 parseColorRGB(const char *str)
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

inline vec4 parseColorRGBA(const char *str)
{
    if (str[0] == '#') // HTML color
    {
        u32 color = std::stoul(str + 1, nullptr, 16);
        // test if color is in the format #RRGGBBAA or #RRGGBB
        if (strlen(str) == 9)
        {
            return vec4((color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF) / 255.0f;
        }
        else
        {
            return vec4((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255) / 255.0f;
        }
    }
    else if (str[0] == 'r' && str[1] == 'g' && str[2] == 'b') // rgb color
    {
        return vec4(parseVec3(str + 5) / 255.0f, 1.0f);
    }
    else if (str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && str[3] == 'a') // rgba color
    {
        return parseVec4(str + 6) / 255.0f;
    }
    else // vec4 color
    {
        return parseVec4(str);
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

struct AABB
{
    const vec3 min;
    const vec3 max;

    AABB(vec3 min, vec3 max) : min(min), max(max)
    {
    }
};

struct OBB
{
    const vec3 center;
    const vec3 halfExtents;
    const mat4 orientation;

    OBB(vec3 center, vec3 halfExtents, mat4 orientation)
        : center(center), halfExtents(halfExtents), orientation(orientation)
    {
    }

    OBB() : center(vec3(0.0f)), halfExtents(vec3(0.0f)), orientation(mat4(1.0f))
    {
    }
};

inline std::unordered_map<std::string, vec4> text2Color = {
    {"BLACK", vec4(0.0f, 0.0f, 0.0f, 1.0f)},         {"WHITE", vec4(1.0f, 1.0f, 1.0f, 1.0f)},
    {"RED", vec4(1.0f, 0.0f, 0.0f, 1.0f)},           {"GREEN", vec4(0.0f, 1.0f, 0.0f, 1.0f)},
    {"BLUE", vec4(0.0f, 0.0f, 1.0f, 1.0f)},          {"YELLOW", vec4(1.0f, 1.0f, 0.0f, 1.0f)},
    {"CYAN", vec4(0.0f, 1.0f, 1.0f, 1.0f)},          {"MAGENTA", vec4(1.0f, 0.0f, 1.0f, 1.0f)},
    {"GRAY", vec4(0.5f, 0.5f, 0.5f, 1.0f)},          {"LIGHT_GRAY", vec4(0.75f, 0.75f, 0.75f, 1.0f)},
    {"DARK_GRAY", vec4(0.25f, 0.25f, 0.25f, 1.0f)},  {"ORANGE", vec4(1.0f, 0.5f, 0.0f, 1.0f)},
    {"PINK", vec4(1.0f, 0.0f, 0.5f, 1.0f)},          {"PURPLE", vec4(0.5f, 0.0f, 1.0f, 1.0f)},
    {"BROWN", vec4(0.6f, 0.3f, 0.0f, 1.0f)},         {"TRANSPARENT", vec4(0.0f, 0.0f, 0.0f, 0.0f)},
    {"LIGHT_BLUE", vec4(0.0f, 1.0f, 1.0f, 1.0f)},    {"LIGHT_GREEN", vec4(0.0f, 1.0f, 0.0f, 1.0f)},
    {"LIGHT_RED", vec4(1.0f, 0.0f, 0.0f, 1.0f)},     {"LIGHT_YELLOW", vec4(1.0f, 1.0f, 0.0f, 1.0f)},
    {"LIGHT_CYAN", vec4(0.0f, 1.0f, 1.0f, 1.0f)},    {"LIGHT_MAGENTA", vec4(1.0f, 0.0f, 1.0f, 1.0f)},
    {"LIGHT_GRAY", vec4(0.75f, 0.75f, 0.75f, 1.0f)}, {"DARK_GRAY", vec4(0.25f, 0.25f, 0.25f, 1.0f)},
    {"LIGHT_ORANGE", vec4(1.0f, 0.5f, 0.0f, 1.0f)},  {"LIGHT_PINK", vec4(1.0f, 0.0f, 0.5f, 1.0f)},
    {"LIGHT_PURPLE", vec4(0.5f, 0.0f, 1.0f, 1.0f)},  {"LIGHT_BROWN", vec4(0.6f, 0.3f, 0.0f, 1.0f)},
};

// source: https://stackoverflow.com/questions/66813961/c-constexpr-constructor-for-colours
// I didn't even know what a user-defined literal was before this
// I simultaneously love and hate C++
// I understand this is basically just a fancy macro, but still
namespace colours
{
struct Colour
{
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;

    constexpr operator glm::vec3() const
    {
        return {r / 255.0f, g / 255.0f, b / 255.0f};
    }
    constexpr operator glm::vec4() const
    {
        return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    }
};

// helper to display values
std::ostream inline &operator<<(std::ostream &os, const Colour &c)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << '{' << std::setw(2) << static_cast<int>(c.r) << ',' << std::setw(2)
        << static_cast<int>(c.g) << ',' << std::setw(2) << static_cast<int>(c.b) << ',' << std::setw(2)
        << static_cast<int>(c.a) << '}';
    return os << oss.str();
}

// decode a nibble
constexpr inline std::uint8_t nibble(char n)
{
    if (n >= '0' && n <= '9')
        return n - '0';
    return n - 'a' + 10;
}

// decode a byte
constexpr inline std::uint8_t byte(const char *b)
{
    return nibble(b[0]) << 4 | nibble(b[1]);
}

// User-defined literals - These don't care if you start with '#' or
// if the strings have the correct length.

constexpr int roff = 1; // offsets in C strings
constexpr int goff = 3;
constexpr int boff = 5;
constexpr int aoff = 7;

namespace literals
{
constexpr Colour inline operator""_rgb(const char *s, std::size_t)
{
    return {byte(s + roff), byte(s + goff), byte(s + boff), 0xff};
}

constexpr Colour inline operator""_rgba(const char *s, std::size_t)
{
    return {byte(s + roff), byte(s + goff), byte(s + boff), byte(s + aoff)};
}
} // namespace literals
using namespace literals;
// constants
// constexpr auto red = "#ff0000"_rgb;
// constexpr auto green = "#00ff00"_rgb;
// constexpr auto blue = "#0000ff"_rgb;
} // namespace colours

using namespace colours::literals;