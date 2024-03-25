#include "shader.hpp"
#include "utils.hpp"
#include "typedef.hpp"
#include "mesh.hpp"
#include "GLutils.hpp"
#include "texture.hpp"
#include "globals.hpp"
#include "gameObject.hpp"

using namespace EngineGlobals;

void glfw_error_callback(i32 error, const char *description)
{
    fprintf(stderr, "GLFW Error: %s\n", description);
}

struct Light
{
    vec3 position;
    vec3 color;
    f32 intensity;
};

// temporary until we have a camera class
vec3 camera_position = vec3(0.0f, 0.0f, 5.0f);
vec3 camera_up = vec3(0.0f, 1.0f, 0.0f);
vec3 camera_target = vec3(0.0f, 0.0f, 0.0f);

// temporary until we have a good system for orbital camera controls
f32 zoom = 5.0f;
vec2 angle_orbit = vec2(0.0f, 0.0f);

void processInput(GLFWwindow *window, f32 deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        // disable cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        f64 xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (clickPos.x == -1.0f)
        {
            clickPos = ivec2(xpos, ypos);
        }

        glfwSetCursorPos(window, clickPos.x, clickPos.y);

        f32 xoffset = clickPos.x - xpos;
        f32 yoffset = clickPos.y - ypos; // reversed since y-coordinates go from bottom to top

        f32 sensitivity = 0.01f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        angle_orbit.x += xoffset;
        angle_orbit.y += yoffset;

        // compute new camera position from angle and zoom (radius)
        camera_position = vec3(
            zoom * cos(angle_orbit.x) * cos(angle_orbit.y),
            zoom * sin(angle_orbit.y),
            zoom * sin(angle_orbit.x) * cos(angle_orbit.y));

        camera_target = -normalize(camera_position);

        viewMatrix = lookAt(camera_position, camera_position + camera_target, camera_up);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        // enable cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        clickPos = vec2(-1.0f, -1.0f);
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    zoom -= yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;

    camera_position = vec3(
        zoom * cos(angle_orbit.x) * cos(angle_orbit.y),
        zoom * sin(angle_orbit.y),
        zoom * sin(angle_orbit.x) * cos(angle_orbit.y));

    camera_target = -normalize(camera_position);

    viewMatrix = lookAt(camera_position, camera_position + camera_target, camera_up);
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

    glfwSetScrollCallback(window, scroll_callback);

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

    // Set up camera
    vec3 cameraPos = vec3(0.0f, 0.0f, -5.0f);
    viewMatrix = lookAt(cameraPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    projectionMatrix = perspective(radians(45.0f), (f32)windowSize.x / (f32)windowSize.y, 0.1f, 100.0f);

    // Load meshes
    // sun
    // load the mesh
    Mesh3DPtr sunMesh = loadMesh3D(programUnlit, "res/sphere_smooth.obj");

    // set the transform
    Transform3D transformSun;
    transformSun.setPosition(vec3(0.0f, 0.0f, 0.0f));
    transformSun.setRotation(vec3(0.0f, 0.0f, 0.0f));
    transformSun.setScale(vec3(2.0f));
    sunMesh->setTransform(transformSun);

    // set the texture
    sunMesh->addTexture("res/2k_sun.jpg");

    // earth
    // load the mesh
    Mesh3DPtr earthMesh = loadMesh3D(programEarth, "res/sphere_smooth.obj");

    // set the transform
    Transform3D transformEarth;
    transformEarth.setPosition(vec3(6.0f, 0.0f, 0.0f));
    transformEarth.setRotation(vec3(0.0f, 0.0f, 0.0f));
    transformEarth.setScale(vec3(1.0f));
    earthMesh->setTransform(transformEarth);

    // set the textures
    earthMesh->addTexture("res/2k_earth_daymap.jpg");
    earthMesh->addTexture("res/2k_earth_nightmap.jpg");

    // moon
    // load the mesh
    Mesh3DPtr moonMesh = loadMesh3D(programPlanet, "res/sphere_smooth.obj");

    // set the transform
    Transform3D transformMoon;
    transformMoon.setPosition(vec3(3.0f, 0.0f, 0.0f));
    transformMoon.setRotation(vec3(0.0f, 0.0f, 0.0f));
    transformMoon.setScale(vec3(0.5f));
    moonMesh->setTransform(transformMoon);

    // set the texture
    moonMesh->addTexture("res/2k_moon.jpg");

    // create game objects
    GameObjectPtr earth = createGameObject(earthMesh);
    GameObjectPtr moon = createGameObject(moonMesh);
    GameObjectPtr sun = createGameObject(sunMesh);

    // set parent-child relationships
    sun->addChild(earth);
    earth->addChild(moon);

    // set uniforms
    mat4 earthModel = earth->getObjectMatrix();
    vec3 earthPosition = earthModel[3];
    earthMesh->setUniform(4, earthPosition);

    mat4 moonModel = moon->getObjectMatrix();
    vec3 moonPosition = moonModel[3];
    moonMesh->setUniform(4, moonPosition);

    u64 i = 0;
    while (!glfwWindowShouldClose(window))
    {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update the sun, earth and moon positions
        Transform3D sunTransform;
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
        earthMesh->setUniform(4, earthPosition);

        mat4 moonModel = moon->getObjectMatrix();
        vec3 moonPosition = moonModel[3];
        moonMesh->setUniform(4, moonPosition);

        // compute deltatime
        f32 currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        std::cout << "FPS: " << 1.0f / deltaTime << "     \r" << std::flush;

        i++;

        // Process input
        processInput(window, deltaTime);

        // draw the scene
        sun->draw();

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}