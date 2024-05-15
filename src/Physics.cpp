#include "Physics.hpp"

#ifndef USE_REACTPHYSICS
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
        f32 impulseMagnitude = glm::dot(-relativeVelocity, collisionNormal) * (1 + collider->material.restitution) /
                               (1 / rigidBody1->getMass() + 1 / rigidBody2->getMass());

        if (rigidBody1->isStatic || rigidBody2->isStatic)
        {
            impulseMagnitude *= 2;
        }

        // Apply the impulse to the rigid bodies
        rigidBody1->addForce(impulseMagnitude * collisionNormal);
        rigidBody2->addForce(-impulseMagnitude * collisionNormal);
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

    // std::cout << std::endl << std::endl;

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

PhysicsEnginePtr getPhysicsEngine()
{
    static PhysicsEnginePtr physicsEngine = std::make_shared<PhysicsEngine>();
    return physicsEngine;
}
#else
namespace rp3d = reactphysics3d;

rp3d::PhysicsCommon &getPhysicsCommon()
{
    static rp3d::PhysicsCommon physicsCommon;
    return physicsCommon;
}

rp3d::PhysicsWorld *getPhysicsWorld()
{
    static rp3d::PhysicsWorld *world = getPhysicsCommon().createPhysicsWorld();
    return world;
}

void PhysicsEngine::Update()
{
    getPhysicsWorld()->update(EngineGlobals::fixedDeltaTime);
}

rp3d::TriangleMesh *toRP3DMesh(const MeshPtr &mesh)
{
    rp3d::TriangleVertexArray triangleArray(mesh->vertices.size(), mesh->vertices.data(), sizeof(glm::vec3),
                                            mesh->normals.data(), sizeof(glm::vec3), mesh->indices.size(),
                                            mesh->indices.data(), sizeof(glm::ivec3),
                                            rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
                                            rp3d::TriangleVertexArray::NormalDataType::NORMAL_FLOAT_TYPE,
                                            rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

    std::vector<rp3d::Message> messages;
    rp3d::TriangleMesh *triangleMesh = getPhysicsCommon().createTriangleMesh(triangleArray, messages);

    constexpr bool showErrors = true;
    constexpr bool showWarnings = false;
    constexpr bool showInfos = false;

    for (auto &message : messages)
    {
        std::string type;
        switch (message.type)
        {
        case rp3d::Message::Type::Error:
            type = "Error";
            break;
        case rp3d::Message::Type::Warning:
            type = "Warning";
            break;
        case rp3d::Message::Type::Information:
            type = "Information";
            break;
        }

        if (message.type == rp3d::Message::Type::Error && showErrors)
        {
            std::cerr << "Error: " << message.text << std::endl;
        }
        else if (message.type == rp3d::Message::Type::Warning && showWarnings)
        {
            std::cerr << "Warning: " << message.text << std::endl;
        }
        else if (message.type == rp3d::Message::Type::Information && showInfos)
        {
            std::cout << "Information: " << message.text << std::endl;
        }
    }

    if (triangleMesh == nullptr)
    {
        std::cerr << "Error: Could not create triangle mesh" << std::endl;
        return nullptr;
    }

    return triangleMesh;
}

PhysicsEnginePtr getPhysicsEngine()
{
    static PhysicsEnginePtr physicsEngine = std::make_shared<PhysicsEngine>();
    return physicsEngine;
}

#endif