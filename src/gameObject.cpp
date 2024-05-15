#include "gameObject.hpp"
#include "mesh.hpp"

GameObjectPtr createGameObject()
{
    return std::make_shared<GameObject>();
}

GameObjectPtr createGameObject(std::string name)
{
    return std::make_shared<GameObject>(name);
}

ComponentPtr GameObject::addComponent(ComponentPtr component)
{
    if (component->getID() == Mesh::getStaticID())
    {
        if (mesh)
        {
            std::cerr << "Error: GameObject already has a mesh component" << std::endl;
            return nullptr;
        }
        mesh = component;
    }

    component->gameObject = shared_from_this();
    components.push_back(component);
    if (started)
    {
        component->Start();
    }
    return component;
}