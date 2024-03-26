#include "camera.hpp"

Camera::Camera()
{
}

Transform3D Camera::getTransform()
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

vec3 Camera::getTarget()
{
    if (needsUpdate)
    {
        updateCamera();
        needsUpdate = false;
    }
    return target;
}

vec3 Camera::getUp()
{
    if (needsUpdate)
    {
        updateCamera();
        needsUpdate = false;
    }
    return up;
}

vec3 Camera::getRight()
{
    if (needsUpdate)
    {
        updateCamera();
        needsUpdate = false;
    }
    return right;
}

vec3 Camera::getForward()
{
    if (needsUpdate)
    {
        updateCamera();
        needsUpdate = false;
    }
    return forward;
}

void Camera::updateCamera()
{
    // compute translation and rotation from object matrix
    vec3 worldTranslation = objMat[3];
    vec3 worldRotation = getEulerAngles(objMat);

    target = worldTranslation + worldRotation * vec3(0.0f, 0.0f, -1.0f);
    up = worldRotation * vec3(0.0f, 1.0f, 0.0f);
    right = worldRotation * vec3(1.0f, 0.0f, 0.0f);
    forward = worldRotation * vec3(0.0f, 0.0f, -1.0f);
    view = glm::lookAt(worldTranslation, target, up);
}

void Camera::lookAt(vec3 _target)
{
    vec3 worldRotation = getEulerAngles(objMat);
}

void Camera::setTransform(Transform3D _transform)
{
    transform = _transform;
}

CameraPtr createCamera()
{
    return std::make_shared<Camera>();
}

namespace EngineGlobals
{
    mat4 getViewMatrix()
    {
        return camera->getView();
    }
}
