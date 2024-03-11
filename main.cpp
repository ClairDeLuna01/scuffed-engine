#include "shader.hpp"
#include "utils.hpp"
#include "typedef.hpp"
#include "mesh.hpp"
#include "GLutils.hpp"

GLFWwindow *window;

int main()
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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1024, 768, "TP1 - GLFW", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
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

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);

    // glEnable(GL_CULL_FACE);

    // get screen size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    u32 SCR_WIDTH = width;
    u32 SCR_HEIGHT = height;

    // update window size
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    ShaderProgramPtr program = std::make_shared<ShaderProgram>("shader/basic.vert", "shader/basic.frag");

    Mesh mesh(program);

    int vaoData[] = {
        0, 1, 2,
        2, 3, 0};

    std::cout << "AAA" << std::endl;

    std::unique_ptr<VertexArrayObject> vao = std::make_unique<VertexArrayObject>(VertexArrayObject((void *)vaoData, 6));

    std::cout << "ZZZ" << std::endl;

    vec3 vertices[] = {
        vec3(-0.5f, -0.5f, 0.0f),
        vec3(0.5f, -0.5f, 0.0f),
        vec3(0.5f, 0.5f, 0.0f),
        vec3(-0.5f, 0.5f, 0.0f)};

    VertexBufferObject vbo1(sizeof(vec3), 0, GL_FLOAT, (void *)vertices, 4);

    std::cout << "AAA" << std::endl;

    mesh.addVBO(vbo1);

    std::cout << "BBB" << std::endl;

    mesh.setVAO(vao);

    std::cout << "CCC" << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mesh.bind();

        mesh.unbind();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
}