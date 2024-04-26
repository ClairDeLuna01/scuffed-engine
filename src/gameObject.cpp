#include "gameObject.hpp"

GameObjectPtr createGameObject()
{
    return std::make_shared<GameObject>();
}

GameObjectPtr createGameObject(std::string name)
{
    return std::make_shared<GameObject>(name);
}
