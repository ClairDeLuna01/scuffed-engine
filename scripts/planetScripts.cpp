#include "UI.hpp"
#include "component.hpp"
#include "gameObject.hpp"
#include "mesh.hpp"

class SunScript : public Script
{
  private:
    [[Serialize]]
    float speed = 0.02f;

  public:
    static UIWindowPtr window;

    SunScript()
    {
    }

    void Start() override
    {
        if (!window)
        {
            window = getUI().add_window("Solar System", {});
        }
        window->add_watcher(gameObject->getName() + " Speed", &speed, UIWindow::WatcherMode::DRAG, -1, 1, 0.01f);
    }

    void Update() override
    {
        gameObject->setTransform(gameObject->getTransform().rotateBy(vec3(0, speed, 0)));
    }

    friend class EarthScript;
    friend class MoonScript;
};

UIWindowPtr SunScript::window = nullptr;

class EarthScript : public Script
{
  private:
    [[Serialize]]
    float speed = 0.015f;
    MeshPtr mesh;
    GameObjectPtr sun;

  public:
    EarthScript()
    {
    }

    void Start() override
    {

        SunScript::window->add_watcher(gameObject->getName() + " Speed", &speed, UIWindow::WatcherMode::DRAG, -1, 1,
                                       0.01f);
        mesh = gameObject->getComponent<Mesh>();
        sun = getGameObject()->getParent();
    }

    void Update() override
    {
        gameObject->setTransform(gameObject->getTransform().rotateBy(vec3(0, speed, 0)));
        mesh->setUniform(UNIFORM_LOCATIONS::VIEW_POS, vec3(gameObject->getObjectMatrix()[3]));
    }
};

class MoonScript : public Script
{
  private:
    [[Serialize]]
    float speed = 0.05f;
    MeshPtr mesh;
    GameObjectPtr sun;

  public:
    MoonScript()
    {
    }

    void Start() override
    {
        SunScript::window->add_watcher(gameObject->getName() + " Speed", &speed, UIWindow::WatcherMode::DRAG, -1, 1,
                                       0.01f);
        mesh = gameObject->getComponent<Mesh>();
        sun = getGameObject()->getParent()->getParent();
    }

    void Update() override
    {
        gameObject->setTransform(gameObject->getTransform().rotateBy(vec3(0, speed, 0)));
        mesh->setUniform(UNIFORM_LOCATIONS::VIEW_POS, vec3(gameObject->getObjectMatrix()[3]));
    }
};
