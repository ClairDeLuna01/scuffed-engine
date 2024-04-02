#include "gameObject.hpp"

GameObjectPtr createGameObject()
{
    return std::make_shared<GameObject>();
}
