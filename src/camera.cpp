#include "camera.hpp"
#include "UI.hpp"
#include "scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
using namespace glm;
using namespace EngineGlobals;

Camera::Camera() : GameObject("Camera")
{
}

Transform3D &Camera::getTransform()
{
    return transform;
}

mat4 Camera::getView()
{
    if (needsUpdate)
    {
        updateCamera();
        needsUpdate = false;
    }
    return view;
}

vec3 Camera::getWorldUp()
{
    return worldUp;
}

void Camera::updateCamera()
{
    // compute translation and rotation from object matrix
    // vec3 worldTranslation = objMat[3];
    // quat worldRotation = quat_cast(objMat);

    // // compute forward vector
    // vec3 forward = worldRotation * vec3(0.0f, 0.0f, -1.0f);

    // vec3 target = worldTranslation + forward;
    // view = glm::lookAt(worldTranslation, target, worldUp);

    view = inverse(objMat);
}

void Camera::lookAt(vec3 _target)
{
    transform.lookAt(_target);
    needsUpdate = true;
}

void Camera::setTransform(Transform3D _transform)
{
    GameObject::setTransform(_transform);
    needsUpdate = true;
}

CameraPtr createCamera()
{
    return std::make_shared<Camera>();
}

void CameraInput::FlyCamera::flyInputKey(GLFWwindow *window, u32 key, u32 scancode, u32 action, u32 mods)
{
    switch (key)
    {
    case GLFW_KEY_W:
        if (action == GLFW_PRESS)
        {
            forward = true;
        }
        if (action == GLFW_RELEASE)
        {
            forward = false;
        }
        break;
    case GLFW_KEY_S:
        if (action == GLFW_PRESS)
        {
            backward = true;
        }
        if (action == GLFW_RELEASE)
        {
            backward = false;
        }
        break;
    case GLFW_KEY_A:
        if (action == GLFW_PRESS)
        {
            left = true;
        }
        if (action == GLFW_RELEASE)
        {
            left = false;
        }
        break;
    case GLFW_KEY_D:
        if (action == GLFW_PRESS)
        {
            right = true;
        }
        if (action == GLFW_RELEASE)
        {
            right = false;
        }
        break;

    case GLFW_KEY_SPACE:
        if (action == GLFW_PRESS)
        {
            up = true;
        }
        if (action == GLFW_RELEASE)
        {
            up = false;
        }
        break;

    case GLFW_KEY_LEFT_SHIFT:
        if (action == GLFW_PRESS)
        {
            down = true;
        }
        if (action == GLFW_RELEASE)
        {
            down = false;
        }
        break;

    default:
        // std::cout << "Key not recognized: " << key << std::endl;
        break;
    }
}

void CameraInput::FlyCamera::flyInputCursor(GLFWwindow *window, f64 xpos, f64 ypos)
{
    // std::cout << camera->transform.getEulerRotation() << std::endl;
    if (lastX == -1)
    {
        lastX = xpos;
        lastY = ypos;
    }
    f32 xoffset = lastX - xpos;
    f32 yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    const vec3 right = camera->transform.getRight();

    quat pitch = angleAxis(yoffset, right);
    quat yaw = angleAxis(xoffset, vec3(0.0f, 1.0f, 0.0f));

    quat rotation = pitch * yaw;

    camera->transform.rotateBy(rotation);

    camera->needsUpdate = true;

    lastX = xpos;
    lastY = ypos;
}

void CameraInput::FlyCamera::flyInputStep(GLFWwindow *window, f32 deltaTime)
{
    using namespace EngineGlobals;
    // std::cout << camera->transform.getPosition() << std::endl;
    if (forward)
    {
        camera->transform.translateBy(camera->transform.getForward() * speed);
        camera->needsUpdate = true;
    }
    if (backward)
    {
        camera->transform.translateBy(-camera->transform.getForward() * speed);
        camera->needsUpdate = true;
    }
    if (left)
    {
        camera->transform.translateBy(-camera->transform.getRight() * speed);
        camera->needsUpdate = true;
    }
    if (right)
    {
        camera->transform.translateBy(camera->transform.getRight() * speed);
        camera->needsUpdate = true;
    }
    if (up)
    {
        camera->transform.translateBy(camera->worldUp * speed);
        camera->needsUpdate = true;
    }
    if (down)
    {
        camera->transform.translateBy(-camera->worldUp * speed);
        camera->needsUpdate = true;
    }

    camera->updateObjectMatrix();
}

void CameraInput::OrbitalCamera::orbitalInputCursor(GLFWwindow *window, f64 xpos, f64 ypos)
{
    if (!mousePressed && needMousePressed)
    {
        return;
    }

    if (lastX == -1)
    {
        lastX = xpos;
        lastY = ypos;
    }

    f32 xoffset = xpos - lastX;
    f32 yoffset = ypos - lastY;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    angleX += xoffset;

    f32 angleLimit = M_PI / 2 * 0.99;
    angleY = clamp(angleY + yoffset, -angleLimit, angleLimit);

    lastX = xpos;
    lastY = ypos;

    // move mouse back to PressX, PressY
    // glfwSetCursorPos(window, pressX, pressY);
}

void CameraInput::OrbitalCamera::orbitalInputStep(GLFWwindow *window, f32 deltaTime)
{
    if (!needMousePressed)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    camera->getTransform().setPosition(
        vec3(radius * cos(angleX) * cos(angleY), radius * sin(angleY), radius * sin(angleX) * cos(angleY)) + offset);
    camera->lookAt(offset);

    camera->needsUpdate = true;
    camera->updateObjectMatrix();

    // scene->getMaterial("BasicMat")->getShader()->setUniform(UNIFORM_LOCATIONS::FOCUS_DISTANCE, radius);
}

void CameraInput::OrbitalCamera::orbitalInputMouse(GLFWwindow *window, u32 button, u32 action, u32 mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !getUI().getAnyWindowHovered())
    {
        mousePressed = true;
        // lock cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        mousePressed = false;
        // unlock cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        lastX = -1;
        lastY = -1;
    }
}

void CameraInput::OrbitalCamera::orbitalInputScroll(GLFWwindow *window, f64 xoffset, f64 yoffset)
{
    if (!mousePressed && needMousePressed)
    {
        return;
    }

    radius -= yoffset;
    if (radius < 1.0f)
    {
        radius = 1.0f;
    }
    camera->transform.setPosition(
        vec3(radius * cos(angleX) * cos(angleY), radius * sin(angleY), radius * sin(angleX) * cos(angleY)));
    camera->lookAt(vec3(0));
    camera->needsUpdate = true;
    camera->updateObjectMatrix();
}

void CameraInput::OrbitalCamera::orbitalInputKey(GLFWwindow *window, u32 key, u32 scancode, u32 action, u32 mods)
{
    switch (key)
    {
    case GLFW_KEY_F2:
        if (action == GLFW_PRESS)
        {
            needMousePressed = !needMousePressed;
            glfwSetInputMode(window, GLFW_CURSOR, needMousePressed ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }
        break;
    default:
        break;
    }
}

namespace EngineGlobals
{
mat4 getViewMatrix()
{
    return camera->getView();
}
} // namespace EngineGlobals

namespace CameraInput
{
f64 lastX = -1, lastY = -1;
f32 sensitivity = 0.005f;
namespace FlyCamera
{
bool forward, backward, left, right, up, down = false;
f32 speed = 0.1f;
} // namespace FlyCamera

namespace OrbitalCamera
{
f32 maxRadius = 15.0f, radius = 15.0f, angleX = 0.0f, angleY = 0.0f;
vec3 offset = vec3(0.0f, 0.0f, 0.0f);
bool mousePressed = false;
bool needMousePressed = false;
} // namespace OrbitalCamera
} // namespace CameraInput
