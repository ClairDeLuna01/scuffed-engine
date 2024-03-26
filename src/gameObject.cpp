#include "gameObject.hpp"

GameObjectPtr createGameObject(Mesh3DPtr mesh)
{
    return std::make_shared<GameObject>(mesh);
}

GameObjectPtr createGameObject()
{
    return std::make_shared<GameObject>();
}
