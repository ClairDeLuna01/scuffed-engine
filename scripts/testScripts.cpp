#include "Physics.hpp"
#include "UI.hpp"
#include "component.hpp"
#include "gameObject.hpp"
#include "inputManager.hpp"
#include "mesh.hpp"
#include "scene.hpp"
#include <reactphysics3d/reactphysics3d.h>

using namespace reactphysics3d;
using namespace glm;

class PhysicsSphere : public Script
{
  public:
    [[Serialize]]
    bool isStatic = false;

    RigidBody *rb;

    void Start() override
    {
        rb = getPhysicsWorld()->createRigidBody(gameObject->getTransform());
        rb->setType(isStatic ? BodyType::STATIC : BodyType::DYNAMIC);
        SphereShape *shape = getPhysicsCommon().createSphereShape(1.0f);
        rb->addCollider(shape, Transform::identity());
    }

    void Update() override
    {
        gameObject->setTransform(rb->getTransform());
    }
};