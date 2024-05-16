#include "DFAUtils.hpp"
#include "GLutils.hpp"
#include "UI.hpp"
#include "camera.hpp"
#include "gameObject.hpp"
#include "globals.hpp"
#include "imgui/imgui.h"
#include "inputManager.hpp"
#include "mesh.hpp"
#include "reactphysics3d/reactphysics3d.h"
#include "scene.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "typedef.hpp"
#include "utils.hpp"

#include "imgui/imgui.h"

using namespace EngineGlobals;
namespace rp3d = reactphysics3d;

i32 main()
{
    // Initialize OpenGL, GLFW and GLEW
    OpenGLInit();

    // Set up projection matrix
    projectionMatrix = perspective(radians(45.0f), (f32)windowSize.x / (f32)windowSize.y, 0.1f, 1000.0f);

    u64 i = 0;

    scene = Scene::Load("scenes/scene.xml");

    getPhysicsWorld()->setGravity(rp3d::Vector3(0.0f, -20.0f, 0.0f));

    f32 fps = 0.0f;

    scene->Start();
    auto w = getUI().add_window("FPS", {});
    w->add_watcher("FPS", &fps, UIWindow::WatcherMode::READONLY);
    while (!glfwWindowShouldClose(window))
    {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        camera->needsUpdate = true;

        // compute deltatime
        f32 currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        fps = 1.0f / deltaTime;

        // std::cout << "FPS: " << 1.0f / deltaTime << "     \r" << std::flush;

        i++;

        // Process escape key input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // draw the scene
        scene->Update();

        getUI().render();

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();

        while (true)
        {
            // cap the framerate to 60fps
            if (glfwGetTime() - currentFrame >= 1.0f / 60.0f)
                break;
        }
    }

    getUI().shutdown();
    glfwTerminate();
    return 0;
}