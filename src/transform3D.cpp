#include "utils.hpp"
#include <iostream>
#include <transform3D.hpp>

vec3 Transform3D::getPosition()
{
    return position;
}

quat Transform3D::getRotation()
{
    return rotation;
}

vec3 Transform3D::getEulerRotation()
{
    return eulerAngles(rotation);
}

vec3 Transform3D::getScale()
{
    return scale;
}

mat4 Transform3D::getModel()
{
    if (modelNeedsUpdate)
    {
        model = mat4(1.0f);
        model = translate(model, position);
        model = model * mat4_cast(rotation);
        model = glm::scale(model, scale);
        modelNeedsUpdate = false;
    }
    return model;
}

Transform3D Transform3D::setPosition(vec3 _position)
{
    position = _position;
    modelNeedsUpdate = true;
    return *this;
}

Transform3D Transform3D::setRotation(quat _rotation)
{
    rotation = normalize(_rotation);
    modelNeedsUpdate = true;
    return *this;
}

Transform3D Transform3D::setRotation(vec3 eulerRotation)
{
    rotation = normalize(quat(eulerRotation));
    modelNeedsUpdate = true;
    return *this;
}

Transform3D Transform3D::setScale(vec3 _scale)
{
    scale = _scale;
    modelNeedsUpdate = true;
    return *this;
}

Transform3D Transform3D::translateBy(vec3 _translation)
{
    position += _translation;
    modelNeedsUpdate = true;
    return *this;
}

Transform3D Transform3D::scaleBy(vec3 _scale)
{
    scale *= _scale;
    modelNeedsUpdate = true;
    return *this;
}

Transform3D Transform3D::rotateBy(quat _rotation)
{
    rotation = normalize(_rotation * rotation);
    modelNeedsUpdate = true;
    return *this;
}

Transform3D Transform3D::lookAt(vec3 _target)
{
    vec3 direction = normalize(_target - position);
    rotation = normalize(quatLookAt(direction, vec3(0.0f, 1.0f, 0.0f)));

    modelNeedsUpdate = true;
    return *this;
}

Transform3D Transform3D::reset()
{
    position = vec3(0.0f);
    rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
    scale = vec3(1.0f);
    modelNeedsUpdate = true;
    return *this;
}

Transform3D &Transform3D::lerp(const Transform3D &target, f32 alpha)
{
    position = glm::lerp(position, target.position, alpha);
    rotation = glm::slerp(rotation, target.rotation, alpha);
    scale = glm::lerp(scale, target.scale, alpha);
    modelNeedsUpdate = true;
    return *this;
}

vec3 Transform3D::getForward()
{
    return rotation * vec3(0.0f, 0.0f, -1.0f);
}

vec3 Transform3D::getRight()
{
    return normalize(cross(getForward(), vec3(0.0f, 1.0f, 0.0f)));
}

vec3 Transform3D::getUp()
{
    return normalize(cross(getRight(), getForward()));
}

void Transform3D::setDirty()
{
    modelNeedsUpdate = true;
}