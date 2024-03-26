#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "transform3D.hpp"
#include "gameObject.hpp"

class Camera : public gameObject
{
private:
    vec3 up = vec3(0.0f, 1.0f, 0.0f);
    vec3 right = vec3(1.0f, 0.0f, 0.0f);
    vec3 forward = vec3(0.0f, 0.0f, -1.0f);
    vec3 target = vec3(0.0f, 0.0f, 0.0f);
    mat4 view = mat4(1.0f);

public:
    bool needsUpdate;
    Camera();

    Transform3D getTransform();

    mat4 getView();

    vec3 getTarget();

    vec3 getUp();
    vec3 getRight();
    vec3 getForward();

    void updateCamera();

    void lookAt(vec3 _target);

    void setTransform(Transform3D _transform);
};