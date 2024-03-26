#include "gameObject.hpp"

GameObjectPtr createGameObject(Mesh3DPtr mesh)
{
    return std::make_shared<gameObject>(mesh);
}

GameObjectPtr createGameObject()
{
    return std::make_shared<gameObject>();
}
