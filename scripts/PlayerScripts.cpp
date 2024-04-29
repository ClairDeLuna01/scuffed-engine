#include "UI.hpp"
#include "component.hpp"
#include "gameObject.hpp"
#include "inputManager.hpp"
#include "mesh.hpp"
#include "scene.hpp"

using namespace glm;

class PlayerController : public Script
{
    bool forward, backward, left, right, shift = false;

    f32 velocity = 0.0f;
    f32 floorHeight = 0.0f;
    bool grounded = false;
    bool jumping = false;
    f32 restitution = 0.0f;
    vec3 randomEulerAngleOffset = vec3(rand(-1.f, 1.f), rand(-1.f, 1.f), rand(-1.f, 1.f)) * 0.1f;
    GameObjectPtr playerMesh;
    GameObjectPtr planeObject;

    keycallback_t keyCallback = [&](GLFWwindow *window, u32 key, u32 scancode, u32 action, u32 mods) {
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_W)
            {
                forward = true;
            }
            if (key == GLFW_KEY_S)
            {
                backward = true;
            }
            if (key == GLFW_KEY_A)
            {
                left = true;
            }
            if (key == GLFW_KEY_D)
            {
                right = true;
            }
            if (key == GLFW_KEY_LEFT_SHIFT)
            {
                shift = true;
            }
            if (key == GLFW_KEY_SPACE && grounded)
            {
                velocity = 0.3f;
                gameObject->setTransform(gameObject->getTransform().translateBy(vec3(0.0f, velocity, 0.0f)));
                grounded = false;
                jumping = true;
                randomEulerAngleOffset = vec3(rand(-1.f, 1.f), rand(-1.f, 1.f), rand(-1.f, 1.f)) * 0.1f;
                randomEulerAngleOffset.x *=
                    forward ? (-2.0f * sign(randomEulerAngleOffset.x)) * (shift ? 2.0f : 1.0f) : 1.0f;
                randomEulerAngleOffset.x *=
                    backward ? (2.0f * sign(randomEulerAngleOffset.x)) * (shift ? 2.0f : 1.0f) : 1.0f;
                randomEulerAngleOffset.z *=
                    right ? (-2.0f * sign(randomEulerAngleOffset.z)) * (shift ? 2.0f : 1.0f) : 1.0f;
                randomEulerAngleOffset.z *=
                    left ? (2.0f * sign(randomEulerAngleOffset.z)) * (shift ? 2.0f : 1.0f) : 1.0f;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            if (key == GLFW_KEY_W)
            {
                forward = false;
            }
            if (key == GLFW_KEY_S)
            {
                backward = false;
            }
            if (key == GLFW_KEY_A)
            {
                left = false;
            }
            if (key == GLFW_KEY_D)
            {
                right = false;
            }
            if (key == GLFW_KEY_LEFT_SHIFT)
            {
                shift = false;
            }
        }
    };

    stepcallback_t stepCallback = [&](GLFWwindow *window, f32 deltaTime) {
        const f32 speed = ((shift) ? 8.0f : 2.0f) * deltaTime;
        if (forward)
        {
            vec3 cameraForward = camera->getTransform().getForward() * vec3(1, 0, 1);
            gameObject->setTransform(gameObject->getTransform().translateBy(cameraForward * speed));
        }
        if (backward)
        {
            vec3 cameraForward = camera->getTransform().getForward() * vec3(1, 0, 1);
            gameObject->setTransform(gameObject->getTransform().translateBy(-cameraForward * speed));
        }
        if (left)
        {
            vec3 cameraRight = camera->getTransform().getRight() * vec3(1, 0, 1);
            gameObject->setTransform(gameObject->getTransform().translateBy(-cameraRight * speed));
        }
        if (right)
        {
            vec3 cameraRight = camera->getTransform().getRight() * vec3(1, 0, 1);
            gameObject->setTransform(gameObject->getTransform().translateBy(cameraRight * speed));
        }

        ::Ray r = {gameObject->getTransform().getPosition() + vec3(0.0f, 100.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)};
        vec3 intersectionPoint;
        vec3 normal;
        if (planeObject->getComponent<LODMesh>()->getCurrentMesh()->meshIntersect(r, intersectionPoint, normal))
        {
            floorHeight = intersectionPoint.y;

            if (!jumping)
                playerMesh->setTransform(
                    playerMesh->getTransform().setRotation(quatLookAt(normal, vec3(0.0f, 1.0f, 0.0f))));
            else
            {
                playerMesh->setTransform(playerMesh->getTransform().rotateBy(
                    quat(randomEulerAngleOffset * max(abs(velocity * 5.0f), 1.0f))));
            }
        }
        else
        {
            if (jumping)
                playerMesh->setTransform(playerMesh->getTransform().rotateBy(
                    quat(randomEulerAngleOffset * max(abs(velocity * 5.0f), 1.0f))));
            else
                playerMesh->setTransform(playerMesh->getTransform().rotateBy(
                    quat(randomEulerAngleOffset * max(abs(velocity * 5.0f), 0.1f))));
            floorHeight = -100.0f;
        }

        const f32 heightOffset = 0.5f;
        if (gameObject->getTransform().getPosition().y <= floorHeight + heightOffset + 0.01f)
        {
            gameObject->setTransform(gameObject->getTransform().setPosition(
                vec3(gameObject->getTransform().getPosition().x, floorHeight + heightOffset,
                     gameObject->getTransform().getPosition().z)));

            // bounce
            if (velocity < 0.0f)
            {
                velocity *= -restitution;
                // std::cout << velocity << std::endl;
                if (abs(velocity) < 0.01f)
                {
                    velocity = 0.0f;
                    grounded = true;
                    jumping = false;
                }

                gameObject->setTransform(gameObject->getTransform().translateBy(vec3(0.0f, velocity, 0.0f)));
            }
            else
            {
                velocity = 0.0f;
                grounded = true;
                jumping = false;
            }

            grounded = true;
            jumping = false;
        }
        else if (gameObject->getTransform().getPosition().y < -90.0f)
        {
            gameObject->setTransform(gameObject->getTransform().setPosition(vec3(0)));
            grounded = false;
        }
        else
        {
            velocity -= 0.01f;
            gameObject->setTransform(gameObject->getTransform().translateBy(vec3(0.0f, velocity, 0.0f)));
            grounded = false;
        }
    };

  public:
    void Start() override
    {
        InputManager::addKeyCallback(keyCallback);
        InputManager::addStepCallback(stepCallback);
        playerMesh = scene->find("PlayerMesh");
        planeObject = scene->find("Terrain");
    }
};