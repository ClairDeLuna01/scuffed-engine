#include "GLutils.hpp"

#define GLFW_DLL
#include <GLFW/glfw3.h>

void glfw_error_callback(i32 error, const char *description)
{
    // if (error == 65537 && glfwWindowShouldClose(window))
    //     return;
    fprintf(stderr, "GLFW Error: %s\n", description);
}

std::ostream &operator<<(std::ostream &os, GLenum_t e)
{
    switch (e.val)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        os << "HIGH";
        break;

    case GL_DEBUG_SEVERITY_MEDIUM:
        os << "MEDIUM";
        break;

    case GL_DEBUG_SEVERITY_LOW:
        os << "LOW";
        break;

    case GL_DEBUG_SEVERITY_NOTIFICATION:
        os << "NOTIFICATION";
        break;

    case GL_DEBUG_TYPE_ERROR:
        os << "ERROR";
        break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        os << "DEPRECATED BEHAVIOR";
        break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        os << "UNDEFINED BEHAVIOR";
        break;

    case GL_DEBUG_TYPE_PORTABILITY:
        os << "UNPORTABLE FUNCTIONALITY";
        break;

    case GL_DEBUG_TYPE_PERFORMANCE:
        os << "POSSIBLE PERFORMANCE ISSUE";
        break;

    case GL_DEBUG_TYPE_MARKER:
        os << "COMMAND STREAM ANNOTATION";
        break;

    case GL_DEBUG_TYPE_PUSH_GROUP:
        os << "GROUP PUSHING";
        break;

    case GL_DEBUG_TYPE_POP_GROUP:
        os << "GROUP POPING";
        break;

    case GL_DEBUG_TYPE_OTHER:
        os << "OTHER TYPE";
        break;

    case GL_DEBUG_SOURCE_API:
        os << "OPENGL API";
        break;

    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        os << "WINDOW-SYSTEM API";
        break;

    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        os << "SHADER COMPILER";
        break;

    case GL_DEBUG_SOURCE_THIRD_PARTY:
        os << "OPENGL ASSOCIATED APPLICATION";
        break;

    case GL_DEBUG_SOURCE_APPLICATION:
        os << "USER";
        break;

    case GL_DEBUG_SOURCE_OTHER:
        os << "OTHER SOURCE";
        break;

    default:
        os << "UNKNOWN";
        break;
    }

    return os;
}

void printGLerror(GLenum _source, GLenum _type, GLuint id, GLenum _severity, GLsizei length, const GLchar *message)
{
    GLenum_t severity(_severity);
    GLenum_t type(_type);
    GLenum_t source(_source);

    const std::string *color = &TERMINAL_RESET;

    if (type.val == GL_DEBUG_TYPE_ERROR)
    {
        color = &TERMINAL_ERROR;
    }
    else if (severity.val == GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        color = &TERMINAL_NOTIF;
    }
    else
    {
        color = &TERMINAL_WARNING;
    }

    std::cerr << TERMINAL_NOTIF << "\nGL CALLBACK " << *color << "[" << type << "]" << TERMINAL_RESET
              << "\n\tid       = " << *color << id << TERMINAL_RESET << "\n\tfrom     = " << *color << source
              << TERMINAL_RESET << "\n\tseverity = " << *color << severity << TERMINAL_RESET
              << "\n\tmessage  = " << *color << message << "\n\n"
              << TERMINAL_RESET;
}

void GLAPIENTRY MessageCallback(GLenum _source, GLenum _type, GLuint id, GLenum _severity, GLsizei length,
                                const GLchar *message, const void *userParam)
{
    /*
        Historic that restrain message callback spam at each frame
        Some non duplicated error could be missed.
    */
#ifdef PREVENT_GL_NOTIF_SPAM
    static std::deque<errorHistoric> historic;
    static const long spamTimeout = 1E4;

    u64 now = GetTimeMs();

    for (auto i = historic.begin(); i != historic.end(); i++)
        if (i->id == id && (now - i->time) < spamTimeout)
            return;

    historic.push_front((errorHistoric){id, now});
#endif

#ifndef SHOW_GL_NOTIF
    if (_severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;
#endif

#ifndef SHOW_GL_WARNING
    if (_severity == GL_DEBUG_SEVERITY_MEDIUM)
        return;
#endif

    printGLerror(_source, _type, id, _severity, length, message);
}

void OpenGLInit()
{
    using namespace EngineGlobals;
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

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    window = glfwCreateWindow(mode->width, mode->height, "Scuffed Engine", glfwGetPrimaryMonitor(), NULL);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);

    glEnable(GL_CULL_FACE);

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, InputManager::keyCallback);
    glfwSetCursorPosCallback(window, InputManager::cursorCallback);
    glfwSetScrollCallback(window, InputManager::scrollCallback);
    glfwSetMouseButtonCallback(window, InputManager::mouseButtonCallback);
    glfwSetWindowSizeCallback(window, InputManager::windowSizeCallback);

    InputManager::addWindowSizeCallback(resizeCallback);

    // get screen size
    i32 width, height;
    glfwGetFramebufferSize(window, &width, &height);
    windowSize = ivec2(width, height);

    // update window size
    glViewport(0, 0, width, height);

    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
}
