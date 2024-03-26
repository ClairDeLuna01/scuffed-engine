#pragma once

#include <iostream>
#include <string>
#include <chrono>

#include <typedef.hpp>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

// HEADER-ONLY
// Collection of utility functions that doesn't fit in other files

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