#include "Physics.hpp"

void PhysicsEngine::handleCollision(ColliderPtr collider, ColliderPtr other)
{
    // Get the rigid bodies associated with the colliders
    RigidBodyPtr rigidBody1 = collider->gameObject->getComponent<RigidBody>();
    RigidBodyPtr rigidBody2 = other->gameObject->getComponent<RigidBody>();

    // Apply collision response based on the collider types
    if (collider->type == ColliderType::SPHERE && other->type == ColliderType::SPHERE)
    {
        // Handle collision between two spheres
        // Calculate the collision normal
        glm::vec3 collisionNormal = glm::normalize(collider->gameObject->getTransform().getPosition() -
                                                   other->gameObject->getTransform().getPosition());

        // Calculate the relative velocity
        glm::vec3 relativeVelocity = rigidBody1->getVelocity() - rigidBody2->getVelocity();

        // Calculate the impulse magnitude
        f32 impulseMagnitude = -(1.0f + collider->getMaterial().restitution) *
                               glm::dot(relativeVelocity, collisionNormal) /
                               (1.0f / rigidBody1->getMass() + 1.0f / rigidBody2->getMass());

        // Apply the impulse to the rigid bodies
        rigidBody1->setVelocity(rigidBody1->getVelocity() + impulseMagnitude / rigidBody1->getMass() * collisionNormal);
        rigidBody2->setVelocity(rigidBody2->getVelocity() - impulseMagnitude / rigidBody2->getMass() * collisionNormal);
    }
    else if (collider->type == ColliderType::SPHERE && other->type == ColliderType::BOX)
    {
        // Handle collision between a sphere and a box
        // TODO: Implement collision response
    }
    else if (collider->type == ColliderType::BOX && other->type == ColliderType::SPHERE)
    {
        // Handle collision between a box and a sphere
        // TODO: Implement collision response
    }
    else if (collider->type == ColliderType::BOX && other->type == ColliderType::BOX)
    {
        // Handle collision between two boxes
        // TODO: Implement collision response
    }
}

void PhysicsEngine::Update()
{
    for (auto &rigidBody : rigidBodies)
    {
        rigidBody->FixedUpdate();
    }

    for (auto &collider : colliders)
    {
        for (auto &other : colliders)
        {
            if (collider != other)
            {
                if (collider->checkCollision(other))
                {
                    handleCollision(collider, other);
                }
            }
        }
    }
}