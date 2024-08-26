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
        auto meshManager = getMeshManager();
        meshManager->addMesh(std::dynamic_pointer_cast<Mesh>(component));
    }

    component->gameObject = shared_from_this();
    components.push_back(component);
    if (started)
    {
        component->Start();
    }
    return component;
}