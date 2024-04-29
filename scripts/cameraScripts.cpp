#include "UI.hpp"
#include "camera.hpp"
#include "component.hpp"
#include "gameObject.hpp"
#include "inputManager.hpp"
#include "mesh.hpp"

class OrbitalCam : public Script
{
    [[Serialize]]
    bool enableGUI = false;

  public:
    void Start() override
    {
        InputManager::addMouseButtonCallback(CameraInput::OrbitalCamera::orbitalInputMouse);
        InputManager::addCursorCallback(CameraInput::OrbitalCamera::orbitalInputCursor);
        InputManager::addScrollCallback(CameraInput::OrbitalCamera::orbitalInputScroll);
        InputManager::addStepCallback(CameraInput::OrbitalCamera::orbitalInputStep);

        if (enableGUI)
        {
            auto window = getUI().add_window("Orbital Camera", {});
            window->add_watcher("radius", &CameraInput::OrbitalCamera::radius, UIWindow::WatcherMode::DRAG, 0, 100,
                                0.1f);
            window->add_watcher("angleX", &CameraInput::OrbitalCamera::angleX, UIWindow::WatcherMode::DRAG, 0, 0, 0.1f);
            f32 angleLimit = M_PI / 2 * 0.99;
            window->add_watcher("angleY", &CameraInput::OrbitalCamera::angleY, UIWindow::WatcherMode::DRAG, -angleLimit,
                                angleLimit, 0.1f);
        }
    }
};
