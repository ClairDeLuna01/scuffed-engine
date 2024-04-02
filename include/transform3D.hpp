#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
using namespace glm;

#include "typedef.hpp"

class Transform3D
{
private:
    vec3 position = vec3(0.0f);
    quat rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
    vec3 scale = vec3(1.0f);

    mat4 model = mat4(1.0f);
    bool modelNeedsUpdate = true;
    bool modelHasChanged = false;

public:
    Transform3D() {}
    Transform3D(vec3 _position, quat _rotation, vec3 _scale) : position(_position), rotation(_rotation), scale(_scale) {}
    Transform3D(vec3 _position, vec3 eulerRotation, vec3 _scale) : position(_position), rotation(quat(eulerRotation)), scale(_scale) {}

    Transform3D setPosition(vec3 _position);

    Transform3D setRotation(quat _rotation);
    Transform3D setRotation(vec3 eulerRotation);

    Transform3D setScale(vec3 _scale);

    vec3 getPosition();

    quat getRotation();
    vec3 getEulerRotation();

    vec3 getScale();

    mat4 getModel();

    Transform3D translateBy(vec3 _translation);

    Transform3D scaleBy(vec3 _scale);

    Transform3D rotateBy(quat _rotation);

    Transform3D lookAt(vec3 _target);

    Transform3D reset();

    vec3 getForward();
    vec3 getRight();
    vec3 getUp();
};
