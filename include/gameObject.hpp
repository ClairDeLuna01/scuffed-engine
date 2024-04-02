#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "shader.hpp"
#include "transform3D.hpp"
#include "texture.hpp"

#include "globals.hpp"
#include "component.hpp"

using namespace EngineGlobals;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace glm;

class GameObject : public std::enable_shared_from_this<GameObject>
{
protected:
    std::vector<GameObjectPtr> children;
    GameObjectPtr parent = nullptr;

    Transform3D transform;
    mat4 objMat = mat4(1.0f);
    bool enabled = true;

    std::vector<ComponentPtr> components;

    friend class Component;

public:
    GameObject() {}

    GameObject(Transform3D _transform) : transform(_transform) {}

    void addChild(GameObjectPtr child)
    {
        children.push_back(child);
        child->parent = shared_from_this();

        updateChildren();
    }

    void setParent(GameObjectPtr _parent)
    {
        parent = _parent;
        parent->addChild(shared_from_this());

        parent->updateChildren();
    }

    GameObjectPtr getParent()
    {
        return parent;
    }

    mat4 getObjectMatrix()
    {
        return objMat;
    }

    void setTransform(Transform3D _transform)
    {
        transform = _transform;

        updateObjectMatrix();
    }

    void updateObjectMatrix()
    {
        objMat = (!parent) ? transform.getModel() : parent->objMat * transform.getModel();
        updateChildren();
    }

    void setEnabled(bool _enabled)
    {
        enabled = _enabled;
    }

    bool getEnabled()
    {
        return enabled;
    }

    void updateChildren()
    {
        for (auto &child : children)
        {
            child->objMat = objMat * child->transform.getModel();
            child->updateChildren();
        }
    }

    void Update()
    {
        if (!enabled)
            return;

        // if (mesh != nullptr)
        // {
        //     // std::cout << objMat << std::endl;
        //     mesh->draw(objMat);
        // }

        for (auto &component : components)
        {
            component->Update();
        }

        for (auto &child : children)
        {
            child->Update();
        }
    }

    Transform3D &getTransform()
    {
        return transform;
    }

    template <typename T, typename... Args, std::enable_if_t<std::is_base_of<Component, T>::value && std::is_constructible<T, Args...>::value, bool> = true>
    std::shared_ptr<T> addComponent(Args... args)
    {
        std::shared_ptr<T> component = std::make_shared<T>(args...);
        component->gameObject = shared_from_this();
        components.push_back(component);
        return component;
    }

    template <typename T, std::enable_if_t<std::is_base_of<Component, T>::value, bool> = true>
    std::shared_ptr<T> getComponent()
    {
        for (auto &component : components)
        {
            if (std::dynamic_pointer_cast<T>(component))
            {
                return std::dynamic_pointer_cast<T>(component);
            }
        }
        return nullptr;
    }

    void Start()
    {
        for (auto &component : components)
        {
            component->Start();
        }
    }

    void EarlyUpdate()
    {
        for (auto &component : components)
        {
            component->EarlyUpdate();
        }
    }

    void LateUpdate()
    {
        for (auto &component : components)
        {
            component->LateUpdate();
        }
    }
};