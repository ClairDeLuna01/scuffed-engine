#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "gameObject.hpp"
#include "transform3D.hpp"

namespace CameraInput
{
extern f64 lastX, lastY;
extern f32 sensitivity;

namespace FlyCamera
{
extern bool forward, backward, left, right, up, down;
extern f32 speed;
void flyInputKey(GLFWwindow *, u32, u32, u32, u32);
void flyInputCursor(GLFWwindow *, f64, f64);
void flyInputStep(GLFWwindow *, f32);
} // namespace FlyCamera

namespace OrbitalCamera
{
extern f32 radius, angleX, angleY;
extern bool mousePressed;
void orbitalInputCursor(GLFWwindow *, f64, f64);
void orbitalInputMouse(GLFWwindow *, u32, u32, u32);
void orbitalInputScroll(GLFWwindow *, f64, f64);
} // namespace OrbitalCamera
} // namespace CameraInput

class Camera : public GameObject
{
  private:
    const vec3 worldUp = vec3(0.0f, 1.0f, 0.0f);
    mat4 view = mat4(1.0f);

  public:
    bool needsUpdate;
    Camera();

    Transform3D getTransform();

    mat4 getView();

    vec3 getWorldUp();

    void updateCamera();

    void lookAt(vec3 _target);

    void setTransform(Transform3D _transform);

    friend void CameraInput::FlyCamera::flyInputKey(GLFWwindow *, u32, u32, u32, u32);
    friend void CameraInput::FlyCamera::flyInputCursor(GLFWwindow *, f64, f64);
    friend void CameraInput::FlyCamera::flyInputStep(GLFWwindow *, f32);

    friend void CameraInput::OrbitalCamera::orbitalInputCursor(GLFWwindow *, f64, f64);
    friend void CameraInput::OrbitalCamera::orbitalInputMouse(GLFWwindow *, u32, u32, u32);
    friend void CameraInput::OrbitalCamera::orbitalInputScroll(GLFWwindow *, f64, f64);
};
