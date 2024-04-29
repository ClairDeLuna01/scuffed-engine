#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "component.hpp"
#include "gameObject.hpp"
#include "globals.hpp"
#include "typedef.hpp"

struct PhysicsMaterial
{
    f32 restitution;
    f32 friction;
};

using PhysicsEnginePtr = std::shared_ptr<class PhysicsEngine>;
using RigidBodyPtr = std::shared_ptr<class RigidBody>;

using ColliderPtr = std::shared_ptr<class Collider>;
using SphereColliderPtr = std::shared_ptr<class SphereCollider>;
using BoxColliderPtr = std::shared_ptr<class BoxCollider>;

enum class ColliderType
{
    SPHERE,
    BOX
};

class PhysicsEngine
{
  private:
    std::vector<RigidBodyPtr> rigidBodies;
    std::vector<ColliderPtr> colliders;

  public:
    void addRigidBody(RigidBodyPtr rigidBody)
    {
        rigidBodies.push_back(rigidBody);
    }

    void addCollider(ColliderPtr collider)
    {
        colliders.push_back(collider);
    }

    void Update();

    void handleCollision(ColliderPtr collider, ColliderPtr other);
};

PhysicsEnginePtr getPhysicsEngine();

class RigidBody : public Component, public std::enable_shared_from_this<RigidBody>
{
  private:
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 force;
    f32 mass;

  public:
    bool isStatic = false;
    RigidBody(f32 mass) : mass(mass)
    {
        if (mass <= 0.0f)
        {
            isStatic = true;
        }
    }

    void Start() override
    {
        getPhysicsEngine()->addRigidBody(shared_from_this());
    }

    void FixedUpdate() override
    {
        if (isStatic)
        {
            // std::cout << "Static object" << std::endl;
            return;
        }

        acceleration = force / mass;
        velocity += acceleration * EngineGlobals::fixedDeltaTime;
        gameObject->setTransform(gameObject->getTransform().translateBy(velocity));

        // std::cout << gameObject->getTransform().getPosition() << std::endl;

        force = glm::vec3(0.0f, -0.005f, 0.0f);
    }

    void addForce(glm::vec3 _force)
    {
        force += _force;
    }

    void setVelocity(glm::vec3 _velocity)
    {
        velocity = _velocity;
    }

    void setAcceleration(glm::vec3 _acceleration)
    {
        acceleration = _acceleration;
    }

    void setMass(f32 _mass)
    {
        mass = _mass;
    }

    glm::vec3 getVelocity()
    {
        return velocity;
    }

    glm::vec3 getAcceleration()
    {
        return acceleration;
    }

    f32 getMass()
    {
        return mass;
    }
};

class Collider : public Component, public std::enable_shared_from_this<Collider>
{
  protected:
    PhysicsMaterial material;
    ColliderType type;

  public:
    Collider(PhysicsMaterial material, ColliderType type) : material(material), type(type)
    {
    }

    void Start() override
    {
        getPhysicsEngine()->addCollider(shared_from_this());
    }

    PhysicsMaterial getMaterial()
    {
        return material;
    }

    void setMaterial(PhysicsMaterial _material)
    {
        material = _material;
    }

    bool checkCollision(ColliderPtr other)
    {
        switch (other->type)
        {
        case ColliderType::SPHERE:
            return checkCollision(std::dynamic_pointer_cast<SphereCollider>(other));
            break;
        case ColliderType::BOX:
            return checkCollision(std::dynamic_pointer_cast<BoxCollider>(other));
            break;
        }
        return false;
    }
    virtual bool checkCollision(SphereColliderPtr other) = 0;
    virtual bool checkCollision(BoxColliderPtr other) = 0;

    friend class PhysicsEngine;
};

class SphereCollider : public Collider
{
  private:
    f32 radius;

  public:
    SphereCollider(f32 radius, PhysicsMaterial material) : Collider(material, ColliderType::SPHERE), radius(radius)
    {
    }

    f32 getRadius()
    {
        return radius;
    }

    void setRadius(f32 _radius)
    {
        radius = _radius;
    }

    bool checkCollision(SphereColliderPtr other) override
    {
        // check collision between two spheres
        return glm::distance(gameObject->getTransform().getPosition(),
                             other->gameObject->getTransform().getPosition()) < radius + other->getRadius();
    }

    bool checkCollision(BoxColliderPtr other) override
    {
        // check collision between a sphere and a box
        return false;
    }
};

class BoxCollider : public Collider
{
  private:
    glm::vec3 size;

  public:
    BoxCollider(glm::vec3 size, PhysicsMaterial material) : Collider(material, ColliderType::BOX), size(size)
    {
    }

    glm::vec3 getSize()
    {
        return size;
    }

    void setSize(glm::vec3 _size)
    {
        size = _size;
    }

    bool checkCollision(SphereColliderPtr other) override
    {
        // check collision between a box and a sphere
        return false;
    }

    bool checkCollision(BoxColliderPtr other) override
    {
        // check collision between two boxes
        return false;
    }
};
