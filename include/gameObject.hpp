#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "shader.hpp"
#include "texture.hpp"
#include "transform3D.hpp"

#include "MeshManager.hpp"
#include "component.hpp"
#include "globals.hpp"
#include "material.hpp"

// using namespace EngineGlobals;

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
    std::string name;
    mat4 objMat = mat4(1.0f);
    bool enabled = true;
    bool started = false;
    mat4 prevMVP = mat4(1.0f);

    std::vector<ComponentPtr> components;

    friend class Component;

  public:
    GameObject() : name("GameObject")
    {
    }

    GameObject(std::string name) : name(name)
    {
    }

    GameObject(Transform3D _transform, std::string name = "") : transform(_transform), name(name)
    {
    }

    void addChild(GameObjectPtr child)
    {
        children.push_back(child);
        if (child->parent)
        {
            child->parent->removeChild(child);
        }

        child->parent = shared_from_this();

        updateChildren();
    }

    void setParent(GameObjectPtr _parent)
    {
        if (parent)
        {
            parent->removeChild(shared_from_this());
        }
        parent = _parent;
        parent->addChild(shared_from_this());

        parent->updateChildren();
    }

    void removeChild(GameObjectPtr child)
    {
        for (auto it = children.begin(); it != children.end(); it++)
        {
            if (*it == child)
            {
                children.erase(it);
                break;
            }
        }
    }

    GameObjectPtr getParent()
    {
        return parent;
    }

    mat4 getObjectMatrix()
    {
        return objMat;
    }

    mat4 getPrevMVP()
    {
        return prevMVP;
    }

    void setPrevMVP(mat4 _prevMVP)
    {
        prevMVP = _prevMVP;
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

    // normal addComponent
    template <
        typename T, typename... Args,
        std::enable_if_t<std::is_base_of<Component, T>::value && std::is_constructible<T, Args...>::value, bool> = true>
    std::shared_ptr<T> addComponent(Args... args)
    {
        std::shared_ptr<T> component = std::make_shared<T>(args...);
        if (std::is_same<T, Mesh>::value)
        {
            MeshManagerPtr meshManager = getMeshManager();
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

    // component ptr addComponent
    ComponentPtr addComponent(ComponentPtr component);

    // could be a problem is args is a string
    ComponentPtr addComponent(std::string name)
    {
        ComponentPtr component = getComponentFactory().createComponent(name);
        if (component)
        {
            component->gameObject = shared_from_this();
            components.push_back(component);
            if (started)
            {
                component->Start();
            }
        }
        return component;
    }

    template <typename T,
              std::enable_if_t<std::is_base_of<Component, T>::value && !std::is_base_of<Script, T>::value, bool> = true>
    std::shared_ptr<T> getComponent()
    {
        for (auto &component : components)
        {
            if (component->getID() == T::getStaticID())
            {
                return std::dynamic_pointer_cast<T>(component);
            }
        }
        return nullptr;
    }

    template <typename T, std::enable_if_t<std::is_base_of<Script, T>::value, bool> = true>
    std::shared_ptr<T> getScript()
    {
        for (auto &component : components)
        {
            if (component->getID() == Script::getStaticID() && std::dynamic_pointer_cast<T>(component) != nullptr)
            {
                return std::dynamic_pointer_cast<T>(component);
            }
        }
        return nullptr;
    }

    void Start()
    {
        started = true;
        for (auto &component : components)
        {
            component->Start();
        }

        for (auto &child : children)
        {
            child->Start();
        }
    }

    void LateStart()
    {
        for (auto &component : components)
        {
            component->LateStart();
        }

        for (auto &child : children)
        {
            child->LateStart();
        }
    }

    void EarlyUpdate()
    {
        for (auto &component : components)
        {
            component->EarlyUpdate();
        }

        for (auto &child : children)
        {
            child->EarlyUpdate();
        }
    }

    void LateUpdate()
    {
        for (auto &component : components)
        {
            component->LateUpdate();
        }

        for (auto &child : children)
        {
            child->LateUpdate();
        }
    }

    void FixedUpdate()
    {
        for (auto &component : components)
        {
            component->FixedUpdate();
        }

        for (auto &child : children)
        {
            child->FixedUpdate();
        }
    }

    void print(u32 depth = 0)
    {
        for (u32 i = 0; i < depth; i++)
        {
            std::cout << "  ";
        }
        std::cout << name << std::endl;
        for (auto &child : children)
        {
            child->print(depth + 1);
        }
    }

    GameObjectPtr find(std::string name)
    {
        if (this->name == name)
        {
            return shared_from_this();
        }
        for (auto &child : children)
        {
            GameObjectPtr found = child->find(name);
            if (found)
            {
                return found;
            }
        }
        return nullptr;
    }

    std::vector<GameObjectPtr> getChildren()
    {
        return children;
    }

    std::string getName()
    {
        return name;
    }
};