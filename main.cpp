#include "shader.hpp"
#include "utils.hpp"
#include "typedef.hpp"
#include "mesh.hpp"
#include "GLutils.hpp"
#include "texture.hpp"
#include "globals.hpp"

using namespace EngineGlobals;

GLFWwindow *window;

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

i32 main()
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
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
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        getchar();
        return -1;
    }

    glfwSetErrorCallback(glfw_error_callback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

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

    ShaderProgramPtr program = std::make_shared<ShaderProgram>("shader/3D.vert", "shader/unlit/texture.frag");

    Mesh3D mesh(program);

    std::vector<uivec3> indices;
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> uvs;

    Mesh3D::FromOBJ("res/sphere.obj", indices, vertices, normals, uvs);

    EBOptr ebo = std::make_unique<ElementBufferObject>(ElementBufferObject((void *)indices.data(), indices.size() * sizeof(uivec3)));

    VertexBufferObject vbo1(3, sizeof(f32), 0, GL_FLOAT, (void *)vertices.data(), vertices.size());
    VertexBufferObject vbo2(3, sizeof(f32), 1, GL_FLOAT, (void *)normals.data(), normals.size());
    VertexBufferObject vbo3(2, sizeof(f32), 2, GL_FLOAT, (void *)uvs.data(), uvs.size());

    mesh.setEBO(ebo);

    mesh.addVBO(vbo1);
    mesh.addVBO(vbo2);
    mesh.addVBO(vbo3);

    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

    Transform3D transform;
    transform.setPosition(vec3(0.0f, 0.0f, 0.0f));
    transform.setRotation(vec3(0.0f, 0.0f, 0.0f));
    transform.setScale(vec3(1.0f));

    vec3 cameraPos = vec3(0.0f, 0.0f, -5.0f);

    viewMatrix = lookAt(cameraPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    projectionMatrix = perspective(radians(45.0f), (f32)windowSize.x / (f32)windowSize.y, 0.1f, 100.0f);

    mesh.setTransform(transform);

    // mesh.setUniform(4, cameraPos);
    // mesh.setUniform(0, vec4(0.84, 0.08, 0.99, 1.0));

    // Light lights[4];
    // lights[0].position = vec3(0, 1.0, 5.0);
    // lights[0].color = vec3(1.0f, 1.0f, 1.0f);
    // lights[0].intensity = 1.0f;

    // lights[1].position = vec3(5.0f, 0, -5.0f);
    // lights[1].color = vec3(1.0f, 1.0f, 1.0f);
    // lights[1].intensity = 0.5f;

    // lights[2].position = vec3(0, 5.0f, -5.0f);
    // lights[2].color = vec3(1.0f, 1.0f, 1.0f);
    // lights[2].intensity = 0.5f;

    TexturePtr texture1 = loadTexture("res/2k_earth_daymap.jpg");

    mesh.addTexture(texture1);

    // for (int i = 0; i < 4; i++)
    // {
    //     mesh.setUniform(1000 + i * 3, lights[i].position);
    //     mesh.setUniform(1001 + i * 3, lights[i].color);
    //     mesh.setUniform(1002 + i * 3, lights[i].intensity);
    // }

    Mesh3D mesh2(program);

    std::vector<uivec3> indices2;
    std::vector<vec3> vertices2;
    std::vector<vec3> normals2;
    std::vector<vec2> uvs2;

    Mesh3D::FromOBJ("res/sphere.obj", indices2, vertices2, normals2, uvs2);

    EBOptr ebo2 = std::make_unique<ElementBufferObject>(ElementBufferObject((void *)indices2.data(), indices2.size() * sizeof(uivec3)));

    VertexBufferObject vbo4(3, sizeof(f32), 0, GL_FLOAT, (void *)vertices2.data(), vertices2.size());
    VertexBufferObject vbo5(3, sizeof(f32), 1, GL_FLOAT, (void *)normals2.data(), normals2.size());
    VertexBufferObject vbo6(2, sizeof(f32), 2, GL_FLOAT, (void *)uvs2.data(), uvs2.size());

    mesh2.setEBO(ebo2);
    mesh2.addVBO(vbo4);
    mesh2.addVBO(vbo5);
    mesh2.addVBO(vbo6);

    Transform3D transform2;
    transform2.setPosition(vec3(4.0f, 0.0f, 0.0f));
    transform2.setRotation(vec3(0.0f, 0.0f, 0.0f));
    transform2.setScale(vec3(0.5f));

    mesh2.setTransform(transform2);
    TexturePtr texture2 = loadTexture("res/2k_moon.jpg");

    mesh2.addTexture(texture2);

    u32 i = 0;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // transform.setRotation(vec3(i / 200.0f, i / 100.0f, i / 50.0f));
        mesh.setTransform(transform);

        f32 currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        i++;

        processInput(window, deltaTime);

        mesh.draw();

        mesh2.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}