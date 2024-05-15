#include "Physics.hpp"
#include "UI.hpp"
#include "component.hpp"
#include "gameObject.hpp"
#include "helper.hpp"
#include "inputManager.hpp"
#include "mesh.hpp"
#include "scene.hpp"
#include <reactphysics3d/reactphysics3d.h>

using namespace reactphysics3d;
using namespace EngineGlobals;
using namespace glm;

// class PhysicsSphere : public Script
// {
//   public:
//     [[Serialize]]
//     bool isStatic = false;

//     RigidBody *rb;

//     void Start() override
//     {
//         rb = getPhysicsWorld()->createRigidBody(gameObject->getTransform());
//         rb->setType(isStatic ? BodyType::STATIC : BodyType::DYNAMIC);
//         SphereShape *shape = getPhysicsCommon().createSphereShape(1.0f);
//         rb->addCollider(shape, Transform::identity());
//     }

//     void Update() override
//     {
//         gameObject->setTransform(rb->getTransform());
//     }
// };

struct PlayerCollisionCallback : public rp3d::CollisionCallback
{
    bool contact = false;
    vec3 normal;
    f32 friction;

    void onContact(const CallbackData &callbackData) override
    {
        contact = true;
        normal = -toVec3(callbackData.getContactPair(0).getContactPoint(0).getWorldNormal());
        f32 friction1 = callbackData.getContactPair(0).getCollider1()->getMaterial().getFrictionCoefficient();
        f32 friction2 = callbackData.getContactPair(0).getCollider2()->getMaterial().getFrictionCoefficient();
        friction = (friction1 + friction2) / 2.0f;
    }
};

quat fromUnitVectors(vec3 from, vec3 to)
{
    f32 r = dot(from, to) + 1.0f;
    if (r < 0.000001f)
    {
        r = 0;
        if (abs(from.x) > abs(from.z))
        {
            return normalize(quat(r, 0, from.y, -from.x));
        }
        else
        {
            return normalize(quat(r, -from.z, from.y, 0));
        }
    }

    quat q = quat(r, from.y * to.z - from.z * to.y, from.z * to.x - from.x * to.z, from.x * to.y - from.y * to.x);
    return normalize(q);
}

class PlayerController : public Script
{
    bool forwardKey, backwardKey, leftKey, rightKey = false;
    reactphysics3d::Collider *col;

    bool special = false;
    bool grounded = false;
    u32 framesSinceGrounded = 0;
    const float bounciness = 0.00f;
    const float bouncinessJump = 0.75f;
    float velocity = 0.0f;
    float angularVelocity = 0.0f;

  public:
    RigidBody *rb;
    PlayerCollisionCallback contact;

    void onInput(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        constexpr f32 jumpForce = 3000.0f;
        if (key == GLFW_KEY_W)
        {
            if (action == GLFW_PRESS)
            {
                forwardKey = true;
            }
            else if (action == GLFW_RELEASE)
            {
                forwardKey = false;
            }
        }
        if (key == GLFW_KEY_S)
        {
            if (action == GLFW_PRESS)
            {
                backwardKey = true;
            }
            else if (action == GLFW_RELEASE)
            {
                backwardKey = false;
            }
        }
        if (key == GLFW_KEY_A)
        {
            if (action == GLFW_PRESS)
            {
                leftKey = true;
            }
            else if (action == GLFW_RELEASE)
            {
                leftKey = false;
            }
        }
        if (key == GLFW_KEY_D)
        {
            if (action == GLFW_PRESS)
            {
                rightKey = true;
            }
            else if (action == GLFW_RELEASE)
            {
                rightKey = false;
            }
        }
        if (key == GLFW_KEY_SPACE)
        {
            if (action == GLFW_PRESS && grounded)
            {
                rb->applyWorldForceAtCenterOfMass(Vector3(0, jumpForce, 0));
                col->getMaterial().setBounciness(bouncinessJump);
            }
        }
        if (key == GLFW_KEY_R)
        {
            if (action == GLFW_PRESS)
            {
                rb->setTransform(Transform::identity());
                rb->setLinearVelocity(Vector3(0, 0, 0));
                rb->setAngularVelocity(Vector3(0, 0, 0));
            }
        }
        if (key == GLFW_KEY_LEFT_SHIFT)
        {
            if (action == GLFW_PRESS && framesSinceGrounded > 0)
            {
                special = true;
            }
        }
    }

    void EarlyUpdate() override
    {
        constexpr float force = 80000.0f;

        Vector3 forward = toVec3(camera->getTransform().getForward() * vec3(1, 0, 1));
        Vector3 right = toVec3(camera->getTransform().getRight() * vec3(1, 0, 1));

        Vector3 inputVec = Vector3(0, 0, 0);
        inputVec += forward * ((i8)forwardKey - (i8)backwardKey);
        inputVec += right * ((i8)rightKey - (i8)leftKey);

        Vector3 forceVec = inputVec * force * 5 * deltaTime;
        vec3 movementRotationAxis = normalize0(cross(vec3(0, 1, 0), toVec3(forceVec)));
        // std::cout << movementRotationAxis << std::endl;

        contact.contact = false;
        getPhysicsWorld()->testCollision(rb, contact);

        if (contact.contact)
        {
            framesSinceGrounded = 0;
            grounded = true;
            // if (special)
            // {
            //     special = false;
            //     rp3d::Vector3 adjustedVel = rb->getLinearVelocity() * rp3d::Vector3(0.1, 1, 0.1);
            //     vec3 v = contact.normal * adjustedVel.length() * 1000.0f;
            //     // std::cout << "v.length: " << length(v) << "          " << rb->getLinearVelocity().length()
            //     //           << "          " << contact.normal << std::endl;
            //     vec3 vel = toVec3(rb->getLinearVelocity());
            //     vel += v / 500.0f;
            //     rb->setLinearVelocity(toVec3(vel));

            //     v = clampLength(v, 0.0f, 5000.0f);

            //     rb->applyWorldForceAtCenterOfMass(toVec3(v));
            // }
            // adapted from https://github.com/Vanilagy/MarbleBlast/blob/master/src/ts/marble.ts

            quat contactNormalRotation = fromUnitVectors(vec3(0, 1, 0), contact.normal);
            movementRotationAxis = rotate(contactNormalRotation, movementRotationAxis);
            // std::cout << movementRotationAxis << std::endl;

            f32 dot1 = dot(-normalize0(toVec3(forceVec)), contact.normal);
            f32 penalty = max(0.0f, dot1 - max(0.0f, col->getMaterial().getFrictionCoefficient() - 1.0f));
            movementRotationAxis *= 1.0f - penalty;
            // std::cout << movementRotationAxis << std::endl;

            vec3 angVel = toVec3(rb->getAngularVelocity());
            vec3 direction = normalize0(movementRotationAxis);
            f32 dot2 = max(0.0f, dot(angVel, movementRotationAxis));
            angVel += direction * -dot2;

            vec3 surfaceRotationAxis = cross(vec3(0, 1, 0), contact.normal);
            f32 dot3 = max(dot(angVel, surfaceRotationAxis), 0.0f);
            angVel += surfaceRotationAxis * -dot3;

            angVel *= pow(0.02f, min(1.0f, contact.friction) * deltaTime);

            angVel += surfaceRotationAxis * dot3;
            angVel += direction * dot2;

            f32 contactNormalUpDot = dot(contact.normal, vec3(0, 1, 0));

            if (length(angVel) > 300)
                angVel *= 300 / length(angVel);

            if (dot2 + length(movementRotationAxis) > 12.0f * M_PI * 2 * inputVec.length() / contactNormalUpDot)
            {
                // std::cout << "dot2: " << dot2 << std::endl;
                // std::cout << length("movementRotationAxis): " << length(movementRotationAxis) << std::endl;
                // std::cout << "inputVec.length(): " << inputVec.length() << std::endl;
                // std::cout << "contactNormalUpDot: " << contactNormalUpDot << std::endl;
                f32 newLength = max(0.0, 12 * M_PI * 2 * inputVec.length() / contactNormalUpDot - dot2);
                movementRotationAxis = normalize0(movementRotationAxis) * newLength;
            }

            // check if player is on the floor and is not touching a wall
            if (contactNormalUpDot > 0.95f)
            {
                col->getMaterial().setBounciness(bounciness);
                // std::cout << "floor" << std::endl;
            }

            // check if player is on a slope
            if (contactNormalUpDot < 0.95 && contactNormalUpDot > 0.1)
            {
                // check if player is going down the slope, if so accelerate
                // get 2D slope direction
                vec3 slopeDirection3D = normalize0(cross(contact.normal, vec3(0, 1, 0)));
                vec2 slopeDirection = vec2(slopeDirection3D.x, slopeDirection3D.z);

                // get 2D player input direction
                vec2 inputDirection = vec2(inputVec.x, inputVec.z);

                // if the player is going down the slope, accelerate
                if (dot(slopeDirection, inputDirection) > 0.0f)
                {
                    vec3 vel = toVec3(rb->getLinearVelocity());
                    f32 veldot = dot(slopeDirection3D, vel);
                    f32 slopeAccel = 0.5f;
                    f32 slopeAccelMag = slopeAccel * deltaTime;
                    f32 slopeAccelMagDot = slopeAccelMag * dot(slopeDirection3D, toVec3(inputVec));
                    f32 slopeAccelMagDotMax = 0.5f;
                    if (slopeAccelMagDot > slopeAccelMagDotMax)
                    {
                        slopeAccelMagDot = slopeAccelMagDotMax;
                    }
                    if (veldot < 0.0f)
                    {
                        slopeAccelMagDot = 0.0f;
                    }
                    vel += slopeDirection3D * slopeAccelMagDot;
                    rb->setLinearVelocity(toVec3(vel));
                }
            }

            // apply downward force to prevent the player from jumping when rolling too fast
            vec3 downwardForce = -contact.normal * rb->getLinearVelocity().length() * 20.0f;

            rb->applyWorldForceAtCenterOfMass(toVec3(downwardForce));

            // std::cout << angVel << std::endl;
            rb->setAngularVelocity(toVec3(angVel));
        }
        else
        {
            if (framesSinceGrounded > 10)
                grounded = false;
            framesSinceGrounded++;
            movementRotationAxis *= 0.25f;

            constexpr f32 airVelocity = 6.2f;
            vec3 airMovementVector = toVec3(inputVec) * airVelocity * deltaTime;
            rb->setLinearVelocity(rb->getLinearVelocity() + toVec3(airMovementVector));
        }

        vec3 augularVelocity = toVec3(rb->getAngularVelocity());
        // std::cout << movementRotationAxis << std::endl << std::endl;
        augularVelocity = clampLength(augularVelocity + movementRotationAxis, -120.0f, 120.0f);
        rb->setAngularVelocity(toVec3(augularVelocity));
    }

    void Start() override
    {
        rb = getPhysicsWorld()->createRigidBody(gameObject->getTransform());
        rb->setMass(5.0f);
        SphereShape *shape = getPhysicsCommon().createSphereShape(gameObject->getTransform().getScale().x);
        col = rb->addCollider(shape, Transform::identity());
        col->getMaterial().setFrictionCoefficient(0.8f);
        col->getMaterial().setBounciness(bounciness);

        InputManager::addKeyCallback(std::bind(&PlayerController::onInput, this, std::placeholders::_1,
                                               std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
                                               std::placeholders::_5));
    }

    void Update() override
    {
        Transform3D t = rb->getTransform();
        Transform3D t0 = gameObject->getTransform();
        t.setScale(t0.getScale());
        t.setPosition(lerp(t.getPosition(), t0.getPosition(), 0.1f));
        gameObject->setTransform(t);

        velocity = length(toVec3(rb->getLinearVelocity()));
        angularVelocity = length(toVec3(rb->getAngularVelocity()));

        f32 fovMin = 45.0f;
        f32 fovMax = 80.0f;
        f32 velFOVStart = 10.0f;
        f32 velFOVEnd = 70.0f;

        f32 fov = lerp(fovMin, fovMax, clamp((velocity - velFOVStart) / (velFOVEnd - velFOVStart), 0.0f, 1.0f));
        EngineGlobals::fov = fov;
        EngineGlobals::refreshProjectionMatrix();

        // scene->getMaterial("BasicMat")->getShader()->setUniform(UNIFORM_LOCATIONS::FOCUS_POSITION, t.getPosition());
    }

    friend class PlayerWrapper;
    friend class PlayerGUI;
    friend class LevelManager;
};

class PlayerWrapper : public Script
{
    GameObjectPtr player;
    std::shared_ptr<PlayerController> playerScript;
    static constexpr int historySize = 5;
    std::array<vec3, historySize> positions;

    void Start() override
    {
        player = gameObject->getChildren()[0];
        playerScript = player->getScript<PlayerController>();
    }

    void EarlyUpdate() override
    {
        if (player)
        {
            // camera->setTransform(camera->getTransform().translateBy(player->getTransform().getPosition()));
            for (int i = historySize - 1; i > 0; i--)
            {
                positions[i] = positions[i - 1];
            }

            positions[0] = player->getTransform().getPosition();

            vec3 currentPos = CameraInput::OrbitalCamera::offset;
            vec3 targetPos = positions[positions.size() - 1];

            CameraInput::OrbitalCamera::offset = lerp(currentPos, targetPos, 0.75f);

            // interpolate look direction with player velocity
            // vec3 moveDir = normalize(toVec3(playerScript->rb->getLinearVelocity()));
        }
    }
};

class PlayerGUI : public Script
{
    GameObjectPtr player;
    std::shared_ptr<PlayerController> playerScript;

    void Start() override
    {
        player = gameObject->getChildren()[0];
        playerScript = player->getScript<PlayerController>();

        auto window = getUI().add_window("GUI", {});
        window->add_watcher("Velocity", &((playerScript.get())->velocity), UIWindow::WatcherMode::READONLY);
        window->add_watcher("Angular Velocity", &playerScript->angularVelocity, UIWindow::WatcherMode::READONLY);
        window->add_watcher("Grounded", &(playerScript->grounded));
        window->add_watcher("", &player->getTransform(), UIWindow::Transform3DWatchFlags::POSITION,
                            UIWindow::WatcherMode::INPUT);
    }
};

class PhysicsTerrain : public Script
{
    rp3d::TriangleMesh *triangleMesh;
    rp3d::RigidBody *rb;

  public:
    void Start() override
    {
        MeshPtr mesh = gameObject->getComponent<Mesh>();
        triangleMesh = toRP3DMesh(mesh);
        ConcaveShape *shape =
            getPhysicsCommon().createConcaveMeshShape(triangleMesh, toVec3(gameObject->getTransform().getScale()));
        rb = getPhysicsWorld()->createRigidBody(gameObject->getTransform());
        rb->setType(BodyType::STATIC);
        auto col = rb->addCollider(shape, Transform::identity());
        col->setIsWorldQueryCollider(true);

        // std::shared_ptr<rp3dTriangleMeshHelper> helper = std::make_shared<rp3dTriangleMeshHelper>(triangleMesh);
        // gameObject->addComponent(helper);
    }

    friend class LevelManager;
};

class LevelManager : public Script
{
    GameObjectPtr level1;
    GameObjectPtr level2;
    GameObjectPtr level3;
    std::shared_ptr<PhysicsTerrain> terrain1;
    std::shared_ptr<PhysicsTerrain> terrain2;
    std::shared_ptr<PhysicsTerrain> terrain3;
    std::shared_ptr<PlayerController> playerScript;

    f32 levelEndRadius = 3.5f;
    f32 levelEndHeight = 1.0f;
    vec3 level1EndPos = vec3(-184.73, -58.63, 56.75) + vec3(0, levelEndHeight, 0);
    f32 level1ResetHeight = -70.0f;
    vec3 level2EndPos = vec3(-31.648, 7.04, -38.28) + vec3(0, levelEndHeight, 0);
    f32 level2ResetHeight = -10.0f;
    vec3 level3EndPos = vec3(-488.5, -55, 6.5) + vec3(0, levelEndHeight, 0);
    f32 level3ResetHeight = -65.0f;

    enum class CurrentLevel
    {
        LEVEL1,
        LEVEL2,
        LEVEL3
    } currentLevel = CurrentLevel::LEVEL1;

    void LateStart() override
    {
        level1 = gameObject->getChildren()[0];
        level2 = gameObject->getChildren()[1];
        level3 = gameObject->getChildren()[2];

        level1->setEnabled(true);
        level2->setEnabled(false);
        level3->setEnabled(false);

        terrain1 = level1->getScript<PhysicsTerrain>();
        terrain2 = level2->getScript<PhysicsTerrain>();
        terrain3 = level3->getScript<PhysicsTerrain>();

        terrain1->rb->setIsActive(true);
        terrain2->rb->setIsActive(false);
        terrain3->rb->setIsActive(false);

        GameObjectPtr player = scene->find("Sphere1");

        playerScript = player->getScript<PlayerController>();
    }

    void Update() override
    {
        switch (currentLevel)
        {
        case CurrentLevel::LEVEL1: {
            f32 dist = distance(toVec3(playerScript->rb->getTransform().getPosition()), level1EndPos);
            if (dist < levelEndRadius)
            {
                level1->setEnabled(false);
                level2->setEnabled(true);
                level3->setEnabled(false);

                terrain1->rb->setIsActive(false);
                terrain2->rb->setIsActive(true);
                terrain3->rb->setIsActive(false);

                playerScript->rb->setTransform(Transform::identity());
                playerScript->rb->setLinearVelocity(Vector3(0, 0, 0));
                playerScript->rb->setAngularVelocity(Vector3(0, 0, 0));

                currentLevel = CurrentLevel::LEVEL2;
            }

            if (playerScript->rb->getTransform().getPosition().y < level1ResetHeight)
            {
                playerScript->rb->setTransform(Transform::identity());
                playerScript->rb->setLinearVelocity(Vector3(0, 0, 0));
                playerScript->rb->setAngularVelocity(Vector3(0, 0, 0));
            }

            break;
        }

        case CurrentLevel::LEVEL2: {
            f32 dist = distance(toVec3(playerScript->rb->getTransform().getPosition()), level2EndPos);
            // std::cout << dist << std::endl;
            if (dist < levelEndRadius)
            {
                level1->setEnabled(false);
                level2->setEnabled(false);
                level3->setEnabled(true);

                terrain1->rb->setIsActive(false);
                terrain2->rb->setIsActive(false);
                terrain3->rb->setIsActive(true);

                playerScript->rb->setTransform(Transform::identity());
                playerScript->rb->setLinearVelocity(Vector3(0, 0, 0));
                playerScript->rb->setAngularVelocity(Vector3(0, 0, 0));

                currentLevel = CurrentLevel::LEVEL3;
            }

            if (playerScript->rb->getTransform().getPosition().y < level2ResetHeight)
            {
                playerScript->rb->setTransform(Transform::identity());
                playerScript->rb->setLinearVelocity(Vector3(0, 0, 0));
                playerScript->rb->setAngularVelocity(Vector3(0, 0, 0));
            }

            break;
        }

        case CurrentLevel::LEVEL3: {
            f32 dist = distance(toVec3(playerScript->rb->getTransform().getPosition()), level3EndPos);
            if (dist < levelEndRadius)
            {
                level1->setEnabled(true);
                level2->setEnabled(false);
                level3->setEnabled(false);

                terrain1->rb->setIsActive(true);
                terrain2->rb->setIsActive(false);
                terrain3->rb->setIsActive(false);

                playerScript->rb->setTransform(Transform::identity());
                playerScript->rb->setLinearVelocity(Vector3(0, 0, 0));
                playerScript->rb->setAngularVelocity(Vector3(0, 0, 0));

                currentLevel = CurrentLevel::LEVEL1;
            }

            if (playerScript->rb->getTransform().getPosition().y < level3ResetHeight)
            {
                playerScript->rb->setTransform(Transform::identity());
                playerScript->rb->setLinearVelocity(Vector3(0, 0, 0));
                playerScript->rb->setAngularVelocity(Vector3(0, 0, 0));
            }

            break;
        }
        }
    }
};