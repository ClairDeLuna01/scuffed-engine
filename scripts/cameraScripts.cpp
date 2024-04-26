#include "UI.hpp"
#include "camera.hpp"
#include "component.hpp"
#include "gameObject.hpp"
#include "inputManager.hpp"
#include "mesh.hpp"

class OrbitalCam : public Script
{
  public:
    void Start() override
    {
        std::cout << "Orbital Camera Script Started" << std::endl;
        InputManager::addMouseButtonCallback(CameraInput::OrbitalCamera::orbitalInputMouse);
        InputManager::addCursorCallback(CameraInput::OrbitalCamera::orbitalInputCursor);
        InputManager::addScrollCallback(CameraInput::OrbitalCamera::orbitalInputScroll);
    }
};