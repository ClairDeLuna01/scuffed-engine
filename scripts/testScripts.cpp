#include "Physics.hpp"
#include "UI.hpp"
#include "component.hpp"
#include "gameObject.hpp"
#include "inputManager.hpp"
#include "mesh.hpp"
#include "scene.hpp"

using namespace glm;

class PhysicsSphere : public Script
{
  public:
    [[Serialize]]
    bool isStatic = false;

    void Start() override
    {
        gameObject->addComponent<RigidBody>(isStatic ? 0.0f : 1.0f);
        constexpr PhysicsMaterial material = {0.5f, 0.5f};
        gameObject->addComponent<SphereCollider>(1.0f, material);
    }
};