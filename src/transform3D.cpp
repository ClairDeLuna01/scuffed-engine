#include <transform3D.hpp>
#include <iostream>

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

void Transform3D::setPosition(vec3 _position)
{
    position = _position;
    modelNeedsUpdate = true;
}

void Transform3D::setRotation(quat _rotation)
{
    rotation = _rotation;
    modelNeedsUpdate = true;
}

void Transform3D::setRotation(vec3 eulerRotation)
{
    rotation = quat(eulerRotation);
    modelNeedsUpdate = true;
}

void Transform3D::setScale(vec3 _scale)
{
    scale = _scale;
    modelNeedsUpdate = true;
}

void Transform3D::translateBy(vec3 _translation)
{
    position += _translation;
    modelNeedsUpdate = true;
}

void Transform3D::scaleBy(vec3 _scale)
{
    scale *= _scale;
    modelNeedsUpdate = true;
}

void Transform3D::rotateBy(quat _rotation)
{
    rotation = _rotation * rotation;
    modelNeedsUpdate = true;
}

void Transform3D::lookAt(vec3 _target)
{
    vec3 direction = normalize(_target - position);
    rotation = quatLookAt(direction, vec3(0.0f, 1.0f, 0.0f));

    modelNeedsUpdate = true;
}

void Transform3D::reset()
{
    position = vec3(0.0f);
    rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
    scale = vec3(1.0f);
    modelNeedsUpdate = true;
}
