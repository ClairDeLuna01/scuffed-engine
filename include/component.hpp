#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <functional>

#include <glm/glm.hpp>

using namespace glm;

typedef std::shared_ptr<class Component> ComponentPtr;

typedef std::shared_ptr<class GameObject> GameObjectPtr;

class Component
{
protected:
    Component() {}
    virtual ~Component() {}

    GameObjectPtr gameObject;

    friend class GameObject;

public:
    virtual void Update() {};
    virtual void EarlyUpdate() {};
    virtual void LateUpdate() {};
    // virtual void FixedUpdate() = 0;
    virtual void Start() {};

    GameObjectPtr getGameObject()
    {
        return gameObject;
    }
};