#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

using namespace glm;

typedef std::shared_ptr<class Component> ComponentPtr;

typedef std::shared_ptr<class GameObject> GameObjectPtr;

// NOT an ECS component
// Just an interface for components which can be attached to GameObjects
class Component
{
  protected:
    Component()
    {
    }
    virtual ~Component()
    {
    }

    GameObjectPtr gameObject;

    friend class GameObject;

  public:
    virtual void Update() {};
    virtual void EarlyUpdate() {};
    virtual void LateUpdate() {};
    virtual void FixedUpdate() {};
    virtual void Start() {};

    GameObjectPtr getGameObject()
    {
        return gameObject;
    }
};

class ComponentFactory
{
  private:
    std::unordered_map<std::string, std::function<ComponentPtr()>> componentMap;

  public:
    template <typename T, std::enable_if_t<std::is_base_of<Component, T>::value, bool> = true>
    void registerComponent(std::string name)
    {
        componentMap[name] = []() -> ComponentPtr { return std::make_shared<T>(); };

        // std::cout << "Component " << name << " registered" << std::endl;
    }

    ComponentPtr createComponent(std::string name)
    {
        if (componentMap.find(name) == componentMap.end())
        {
            std::cerr << "Error: Component " << name << " not found" << std::endl;
            return nullptr;
        }

        return componentMap[name]();
    }
};

ComponentFactory &getComponentFactory();

// Macro for component registration
#define REGISTER_SCRIPT(TYPE)                                                                                          \
    namespace                                                                                                          \
    {                                                                                                                  \
    struct _AutoRegister_##TYPE                                                                                        \
    {                                                                                                                  \
        _AutoRegister_##TYPE()                                                                                         \
        {                                                                                                              \
            getComponentFactory().registerComponent<TYPE>(#TYPE);                                                      \
        }                                                                                                              \
    };                                                                                                                 \
    static _AutoRegister_##TYPE _autoRegister_##TYPE;                                                                  \
    }

// Property registration and value setting macro
template <typename T> bool setProperty(const std::string &value, T &property);

#define REGISTER_PROPERTY(TYPE, PROPERTY)                                                                              \
    if (__name == #PROPERTY)                                                                                           \
    {                                                                                                                  \
        setProperty<TYPE>(__value, PROPERTY);                                                                          \
        return true;                                                                                                   \
    }

class Script : public Component
{
  protected:
    Script()
    {
    }
    virtual ~Script()
    {
    }

  public:
    // maybe look into templating this later if possible idk
    // okay past me how would that even work
    virtual bool Serialize(const std::string &__name, const std::string &__value) = 0;
};