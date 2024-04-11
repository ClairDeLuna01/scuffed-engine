#include "shader.hpp"
#include "utils.hpp"
#include "typedef.hpp"
#include "mesh.hpp"
#include "GLutils.hpp"
#include "texture.hpp"
#include "globals.hpp"
#include "gameObject.hpp"
#include "camera.hpp"
#include "inputManager.hpp"
#include "DFAUtils.hpp"
#include "UI.hpp"

#include "imgui/imgui.h"

using namespace EngineGlobals;

i32 main()
{
    // Initialize OpenGL, GLFW and GLEW
    OpenGLInit();

    // Load shaders
    ShaderProgramPtr programUnlit = std::make_shared<ShaderProgram>("shader/3D.vert", "shader/unlit/texture.frag"); // unlit shader for the sun
    ShaderProgramPtr programPlanet = std::make_shared<ShaderProgram>("shader/3D.vert", "shader/lit/planet.frag");   // lit shader for the moon that has only one texture and computes lighting
    ShaderProgramPtr programEarth = std::make_shared<ShaderProgram>("shader/3D.vert", "shader/lit/earth.frag");     // lit shader for the earth that has two textures, day and night
    ShaderProgramPtr programLit = std::make_shared<ShaderProgram>("shader/3D.vert", "shader/lit/basic.frag");       // lit shader for the plane

    // Set up projection matrix
    projectionMatrix = perspective(radians(45.0f), (f32)windowSize.x / (f32)windowSize.y, 0.1f, 1000.0f);

    // create game objects
    GameObjectPtr earth = createGameObject();
    GameObjectPtr moon = createGameObject();
    GameObjectPtr sun = createGameObject();

    // Load meshes
    sun->addComponent<Mesh>(programUnlit, "res/sphere_smooth.obj")
        ->addTexture("res/2k_sun.jpg");

    earth->addComponent<Mesh>(programEarth, "res/sphere_smooth.obj")
        ->addTexture("res/2k_earth_daymap.jpg")
        ->addTexture("res/2k_earth_nightmap.jpg");

    moon->addComponent<Mesh>(programPlanet, "res/sphere_smooth.obj")
        ->addTexture("res/2k_moon.jpg");

    // set the transform
    Transform3D sunTransform;
    sunTransform.setPosition(vec3(0.0f, 10.0f, 0.0f));
    sunTransform.setScale(vec3(2.0f));
    sun->setTransform(sunTransform);

    Transform3D earthTransform;
    earthTransform.setPosition(vec3(6.0f, 0.0f, 0.0f));
    earthTransform.setScale(vec3(0.75f));
    earth->setTransform(earthTransform);

    Transform3D moonTransform;
    moonTransform.setPosition(vec3(2.0f, 0.0f, 0.0f));
    moonTransform.setScale(vec3(0.3f));
    moon->setTransform(moonTransform);

    // set parent-child relationships
    sceneRoot->addChild(sun);
    sun->addChild(earth);
    earth->addChild(moon);

    Transform3D cameraTransform;
    vec3 cameraPos = vec3(0.0f, 3.0f, 15.0f);
    cameraTransform.setPosition(cameraPos);
    // cameraTransform.lookAt(vec3(0));
    // cameraTransform.setRotation(vec3(0, -1, 0));
    camera->setTransform(cameraTransform);

    // set uniforms
    mat4 earthModel = earth->getObjectMatrix();
    vec3 earthPosition = earthModel[3];
    MeshPtr earthMesh = earth->getComponent<Mesh>();
    earthMesh->setUniform(UNIFORM_LOCATIONS::VIEW_POS, earthPosition);
    earthMesh->setUniform(5, sun->getTransform().getPosition());

    mat4 moonModel = moon->getObjectMatrix();
    vec3 moonPosition = moonModel[3];
    MeshPtr moonMesh = moon->getComponent<Mesh>();
    moonMesh->setUniform(UNIFORM_LOCATIONS::VIEW_POS, moonPosition);
    moonMesh->setUniform(5, sun->getTransform().getPosition());

    ShaderProgramPtr skyboxShader = std::make_shared<ShaderProgram>("shader/skybox.vert", "shader/skybox.frag");
    CubeMapPtr cubeMap = loadCubeMap(std::array<std::string, 6>({"res/Daylight Box_Right.bmp",
                                                                 "res/Daylight Box_Left.bmp",
                                                                 "res/Daylight Box_Top.bmp",
                                                                 "res/Daylight Box_Bottom.bmp",
                                                                 "res/Daylight Box_Front.bmp",
                                                                 "res/Daylight Box_Back.bmp"}));

    // CubeMapPtr cubeMap = loadCubeMap("res/cubemaps_skybox.png");

    // InputManager::addKeyCallback(CameraInput::FlyCamera::flyInputKey);
    // InputManager::addCursorCallback(CameraInput::FlyCamera::flyInputCursor);
    // InputManager::addStepCallback(CameraInput::FlyCamera::flyInputStep);

    InputManager::addCursorCallback(CameraInput::OrbitalCamera::orbitalInputCursor);
    InputManager::addMouseButtonCallback(CameraInput::OrbitalCamera::orbitalInputMouse);
    InputManager::addScrollCallback(CameraInput::OrbitalCamera::orbitalInputScroll);

    SkyboxPtr skybox = loadSkybox(skyboxShader, cubeMap);

    GameObjectPtr planeObject = createGameObject();
    std::vector<MeshPtr> planeMeshes;
    planeMeshes.push_back(loadMesh(programLit, "res/plane.obj"));
    planeMeshes.push_back(loadMesh(programLit, "res/plane.lod1.obj"));
    planeMeshes.push_back(loadMesh(programLit, "res/plane.lod2.obj"));
    planeMeshes.push_back(loadMesh(programLit, "res/plane.lod3.obj"));

    std::vector<f32> distances = {25.0f, 35.0f, 45.0f};
    planeObject->addComponent<LODMesh>(planeMeshes, distances);

    // planeObject->addComponent<Mesh>(programLit, "res/plane.obj");

    GameObjectPtr player = createGameObject();
    GameObjectPtr playerMesh = createGameObject();
    playerMesh->addComponent<Mesh>(programLit, "res/cube.obj");
    playerMesh->getTransform().setScale(vec3(0.3f));

    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool shift = false;

    f32 playerVelocity = 0.0f;
    f32 floorHeight = 0.0f;
    bool grounded = false;
    bool jumping = false;
    f32 restitution = 0.5f;
    vec3 randomEulerAngleOffset = vec3(rand(-1.f, 1.f), rand(-1.f, 1.f), rand(-1.f, 1.f)) * 0.1f;

    keycallback_t playerKeyCallback = [&](GLFWwindow *window, u32 key, u32 scancode, u32 action, u32 mods)
    {
        // std::cout << "Key: " << key << " Action: " << action << std::endl;
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_W)
            {
                forward = true;
            }
            if (key == GLFW_KEY_S)
            {
                backward = true;
            }
            if (key == GLFW_KEY_A)
            {
                left = true;
            }
            if (key == GLFW_KEY_D)
            {
                right = true;
            }
            if (key == GLFW_KEY_LEFT_SHIFT)
            {
                shift = true;
            }
            if (key == GLFW_KEY_SPACE && grounded)
            {
                playerVelocity = 0.3f;
                player->setTransform(player->getTransform().translateBy(vec3(0.0f, playerVelocity, 0.0f)));
                grounded = false;
                jumping = true;
                randomEulerAngleOffset = vec3(rand(-1.f, 1.f), rand(-1.f, 1.f), rand(-1.f, 1.f)) * 0.1f;
                randomEulerAngleOffset.x *= forward ? (-2.0f * sign(randomEulerAngleOffset.x)) * (shift ? 2.0f : 1.0f) : 1.0f;
                randomEulerAngleOffset.x *= backward ? (2.0f * sign(randomEulerAngleOffset.x)) * (shift ? 2.0f : 1.0f) : 1.0f;
                randomEulerAngleOffset.z *= right ? (-2.0f * sign(randomEulerAngleOffset.z)) * (shift ? 2.0f : 1.0f) : 1.0f;
                randomEulerAngleOffset.z *= left ? (2.0f * sign(randomEulerAngleOffset.z)) * (shift ? 2.0f : 1.0f) : 1.0f;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            if (key == GLFW_KEY_W)
            {
                forward = false;
            }
            if (key == GLFW_KEY_S)
            {
                backward = false;
            }
            if (key == GLFW_KEY_A)
            {
                left = false;
            }
            if (key == GLFW_KEY_D)
            {
                right = false;
            }
            if (key == GLFW_KEY_LEFT_SHIFT)
            {
                shift = false;
            }
        }
    };

    stepcallback_t playerStepCallback = [&](GLFWwindow *window, f32 deltaTime)
    {
        const f32 speed = ((shift) ? 8.0f : 2.0f) * deltaTime;
        if (forward)
        {
            vec3 cameraForward = camera->getTransform().getForward() * vec3(1, 0, 1);
            player->setTransform(player->getTransform().translateBy(cameraForward * speed));
        }
        if (backward)
        {
            vec3 cameraForward = camera->getTransform().getForward() * vec3(1, 0, 1);
            player->setTransform(player->getTransform().translateBy(-cameraForward * speed));
        }
        if (left)
        {
            vec3 cameraRight = camera->getTransform().getRight() * vec3(1, 0, 1);
            player->setTransform(player->getTransform().translateBy(-cameraRight * speed));
        }
        if (right)
        {
            vec3 cameraRight = camera->getTransform().getRight() * vec3(1, 0, 1);
            player->setTransform(player->getTransform().translateBy(cameraRight * speed));
        }
    };

    sceneRoot->addChild(planeObject);
    sceneRoot->addChild(player);
    player->addChild(camera);
    player->addChild(playerMesh);

    stepcallback_t testCallback = [&](GLFWwindow *window, f32 deltaTime)
    {
        Ray r = {player->getTransform().getPosition() + vec3(0.0f, 100.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)};
        vec3 intersectionPoint;
        vec3 normal;
        if (planeObject->getComponent<LODMesh>()->getCurrentMesh()->meshIntersect(r, intersectionPoint, normal))
        {
            floorHeight = intersectionPoint.y;

            if (!jumping)
                playerMesh->setTransform(playerMesh->getTransform().setRotation(quatLookAt(normal, vec3(0.0f, 1.0f, 0.0f))));
            else
            {
                playerMesh->setTransform(playerMesh->getTransform().rotateBy(quat(randomEulerAngleOffset * max(abs(playerVelocity * 5.0f), 1.0f))));
            }
        }
        else
        {
            if (jumping)
                playerMesh->setTransform(playerMesh->getTransform().rotateBy(quat(randomEulerAngleOffset * max(abs(playerVelocity * 5.0f), 1.0f))));
            else
                playerMesh->setTransform(playerMesh->getTransform().rotateBy(quat(randomEulerAngleOffset * max(abs(playerVelocity * 5.0f), 0.1f))));
            floorHeight = -100.0f;
        }

        const f32 heightOffset = 0.5f;
        if (player->getTransform().getPosition().y <= floorHeight + heightOffset + 0.01f)
        {
            player->setTransform(player->getTransform().setPosition(vec3(
                player->getTransform().getPosition().x,
                floorHeight + heightOffset,
                player->getTransform().getPosition().z)));

            // bounce
            if (playerVelocity < 0.0f)
            {
                playerVelocity *= -restitution;
                // std::cout << playerVelocity << std::endl;
                if (abs(playerVelocity) < 0.01f)
                {
                    playerVelocity = 0.0f;
                    grounded = true;
                    jumping = false;
                }

                player->setTransform(player->getTransform().translateBy(vec3(0.0f, playerVelocity, 0.0f)));
            }
            else
            {
                playerVelocity = 0.0f;
                grounded = true;
                jumping = false;
            }

            grounded = true;
            jumping = false;
        }
        else if (player->getTransform().getPosition().y < -90.0f)
        {
            player->setTransform(player->getTransform().setPosition(vec3(0)));
            grounded = false;
        }
        else
        {
            playerVelocity -= 0.01f;
            player->setTransform(player->getTransform().translateBy(vec3(0.0f, playerVelocity, 0.0f)));
            grounded = false;
        }
    };

    InputManager::addStepCallback(testCallback);
    InputManager::addKeyCallback(playerKeyCallback);
    InputManager::addStepCallback(playerStepCallback);

    Light lights[4];
    memset(lights, 0, sizeof(lights));
    lights[0].position = vec3(0.0f, 10.0f, 0.0f);
    lights[0].color = rgb(255, 255, 255);
    lights[0].intensity = 0.5f;

    lights[1].position = vec3(5.0f, 5.0f, 0.0f);
    lights[1].color = rgb(91, 206, 250);
    lights[1].intensity = 1.0f;

    lights[2].position = vec3(-5.0f, 5.0f, 0.0f);
    lights[2].color = rgb(245, 169, 184);
    lights[2].intensity = 1.0f;

    // i32 positionLoc = programLit->getUniformLocation("lights[0].position");
    // i32 colorLoc = programLit->getUniformLocation("lights[0].color");
    // i32 intensityLoc = programLit->getUniformLocation("lights[0].intensity");

    programLit->use();
    for (int i = 0; i < 4; i++)
    {
        programLit->setUniform(LIGHT_POSITION + i * 3, lights[i].position);
        programLit->setUniform(LIGHT_POSITION + i * 3 + 1, lights[i].color);
        programLit->setUniform(LIGHT_POSITION + i * 3 + 2, lights[i].intensity);
    }

    MeshPtr planeMesh = planeObject->getComponent<Mesh>();

    u64 i = 0;

    DFATransition<0, 1, 1> transition1;
    DFATransition<0, 2, 0> transition2;
    DFATransition<1, 2, 1> transition3;
    DFATransition<1, 1, 0> transition4;
    DFATransition<1, 0, 0> transition5;

    DFA dfa(
        0,
        std::vector<i32>({0}),
        transition1,
        transition2,
        transition3,
        transition4,
        transition5);

    // std::cout << (dfa.run(std::vector<i32>({2, 2, 1, 1, 1, 1, 2})) ? "Accepted" : "Rejected") << std::endl;
    // dfa.printTransitionMap();

    ui = new UI();

    UIWindowPtr w = ui->add_window("sunRotation", {});
    w->add_watcher("sunRotation", &sun->getTransform(), UIWindow::Transform3DWatchFlags::ROTATION);

    // UIWindowPtr w = ui->add_window("test", {[](void)
    //                                         {
    //                                             ImGui::Text("Hello, world!");
    //                                             // ImGui::SliderFloat("Slider", &sun->getTransform().rotation.y, 0.0f, 6.28318530718f);
    //                                         }});

    sceneRoot->Start();
    while (!glfwWindowShouldClose(window))
    {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera->needsUpdate = true;

        // Update the sun, earth and moon positions
        Transform3D sunTransform = sun->getTransform();
        // sunTransform.setRotation(vec3(0, i / 100.0f, 0.0f));
        sun->setTransform(sunTransform);

        Transform3D earthTransform = earth->getTransform();
        earthTransform.setRotation(vec3(0.0f, i / 50.0f, radians(22.44f)));
        earth->setTransform(earthTransform);

        Transform3D moonTransform = moon->getTransform();
        moonTransform.setRotation(vec3(0.0f, i / 25.0f, radians(6.68f)));
        moon->setTransform(moonTransform);

        mat4 earthModel = earth->getObjectMatrix();
        vec3 earthPosition = earthModel[3];
        earthMesh->setUniform(UNIFORM_LOCATIONS::VIEW_POS, earthPosition);

        mat4 moonModel = moon->getObjectMatrix();
        vec3 moonPosition = moonModel[3];
        moonMesh->setUniform(UNIFORM_LOCATIONS::VIEW_POS, moonPosition);

        // Transform3D cameraTargetTransform = camera->getTransform();
        // cameraTargetTransform.setPosition(vec3(15.0f * sin(i / 200.0f), 3.0f, 15.0f * cos(i / 200.0f)));
        // cameraTargetTransform.lookAt(vec3(0));
        // cameraTargetTransform.setRotation(vec3(0, i / 100.0f, 0.0f));
        // camera->setTransform(cameraTargetTransform);

        // compute deltatime
        f32 currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        std::cout << "FPS: " << 1.0f / deltaTime << "     \r" << std::flush;

        i++;

        // Process escape key input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        InputManager::stepCallback(window, deltaTime);

        // draw the scene
        sceneRoot->EarlyUpdate();
        sceneRoot->Update();
        sceneRoot->LateUpdate();

        if (skybox)
        {
            skybox->draw();
        }

        ui->render();

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}