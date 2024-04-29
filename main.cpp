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

i32 main()
{
    // Initialize OpenGL, GLFW and GLEW
    OpenGLInit();

    // Set up projection matrix
    projectionMatrix = perspective(radians(45.0f), (f32)windowSize.x / (f32)windowSize.y, 0.1f, 1000.0f);

    u64 i = 0;

    scene = Scene::Load("scenes/scene.xml");

    getPhysicsWorld()->setGravity(reactphysics3d::Vector3(0.0f, -9.81f, 0.0f));

    scene->Start();
    while (!glfwWindowShouldClose(window))
    {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera->needsUpdate = true;

        // compute deltatime
        f32 currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        std::cout << "FPS: " << 1.0f / deltaTime << "     \r" << std::flush;

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
    }

    getUI().shutdown();
    glfwTerminate();
    return 0;
}