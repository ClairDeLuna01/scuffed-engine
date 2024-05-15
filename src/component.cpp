#include "component.hpp"

#include <iostream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

ComponentFactory &getComponentFactory()
{
    static ComponentFactory factory;
    return factory;
}

template <> bool setProperty<bool>(const std::string &value, bool &property)
{
    property = value == "true";
    return true;
}

template <> bool setProperty<int>(const std::string &value, int &property)
{
    property = std::stoi(value);
    return true;
}

template <> bool setProperty<float>(const std::string &value, float &property)
{
    property = std::stof(value);
    return true;
}

template <> bool setProperty<std::string>(const std::string &value, std::string &property)
{
    property = value;
    return true;
}

template <> bool setProperty<glm::vec2>(const std::string &value, glm::vec2 &property)
{
    std::stringstream ss(value);
    ss >> property.x >> property.y;
    return true;
}

template <> bool setProperty<glm::vec3>(const std::string &value, glm::vec3 &property)
{
    std::stringstream ss(value);
    ss >> property.x >> property.y >> property.z;
    return true;
}

template <> bool setProperty<glm::vec4>(const std::string &value, glm::vec4 &property)
{
    std::stringstream ss(value);
    ss >> property.x >> property.y >> property.z >> property.w;
    return true;
}

template <> bool setProperty<glm::quat>(const std::string &value, glm::quat &property)
{
    std::stringstream ss(value);
    ss >> property.x >> property.y >> property.z >> property.w;
    return true;
}