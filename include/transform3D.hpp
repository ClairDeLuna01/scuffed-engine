#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
using namespace glm;

#include <reactphysics3d/reactphysics3d.h>

namespace rp3d = reactphysics3d;

#include "typedef.hpp"
#include "utils.hpp"

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
    Transform3D(vec3 _position = vec3(0.0f), quat _rotation = quat(1.0f, 0.0f, 0.0f, 0.0f), vec3 _scale = vec3(1.0f))
        : position(_position), rotation(_rotation), scale(_scale)
    {
    }
    Transform3D(vec3 _position, vec3 eulerRotation, vec3 _scale)
        : position(_position), rotation(quat(eulerRotation)), scale(_scale)
    {
    }
    Transform3D(const rp3d::Transform &t) : position(toVec3(t.getPosition())), rotation(toQuat(t.getOrientation()))
    {
    }

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

    Transform3D &lerp(const Transform3D &target, f32 alpha);

    operator rp3d::Transform() const
    {
        return rp3d::Transform(toVec3(position), toQuat(rotation));
    }

    vec3 getForward();
    vec3 getRight();
    vec3 getUp();

    void setDirty();
};
