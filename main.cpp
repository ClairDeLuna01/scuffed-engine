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

using namespace EngineGlobals;

void glfw_error_callback(i32 error, const char *description)
{
    fprintf(stderr, "GLFW Error: %s\n", description);
}

inline void resizeCallback(GLFWwindow *window, i32 width, i32 height)
{
    windowSize = ivec2(width, height);
    glViewport(0, 0, width, height);
    projectionMatrix = perspective(radians(45.0f), (f32)width / (f32)height, 0.1f, 100.0f);
}

void OpenGLInit()
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1024, 768, "Scuffed Engine", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        getchar();
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(glfw_error_callback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Enable V-Sync
    glfwSwapInterval(1);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);

    // glEnable(GL_CULL_FACE);

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, InputManager::keyCallback);
    glfwSetCursorPosCallback(window, InputManager::cursorCallback);
    glfwSetScrollCallback(window, InputManager::scrollCallback);
    glfwSetMouseButtonCallback(window, InputManager::mouseButtonCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);

    // get screen size
    i32 width, height;
    glfwGetFramebufferSize(window, &width, &height);
    windowSize = ivec2(width, height);

    // update window size
    glViewport(0, 0, width, height);

    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
}

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
    projectionMatrix = perspective(radians(45.0f), (f32)windowSize.x / (f32)windowSize.y, 0.1f, 100.0f);

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

    sceneRoot->addChild(camera);

    // set uniforms
    mat4 earthModel = earth->getObjectMatrix();
    vec3 earthPosition = earthModel[3];
    MeshPtr earthMesh = earth->getComponent<Mesh>();
    earthMesh->setUniform(UNIFORM_LOCATIONS::VIEW_POS, earthPosition);

    mat4 moonModel = moon->getObjectMatrix();
    vec3 moonPosition = moonModel[3];
    MeshPtr moonMesh = moon->getComponent<Mesh>();
    moonMesh->setUniform(UNIFORM_LOCATIONS::VIEW_POS, moonPosition);

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
    planeObject->addComponent<Mesh>(programLit, "res/plane.obj");

    sceneRoot->addChild(planeObject);

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

    sceneRoot->Start();
    while (!glfwWindowShouldClose(window))
    {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera->needsUpdate = true;

        // Update the sun, earth and moon positions
        Transform3D sunTransform = sun->getTransform();
        sunTransform.setRotation(vec3(0, i / 100.0f, 0.0f));
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

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}