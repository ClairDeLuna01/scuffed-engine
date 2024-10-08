#include "texture.hpp"
#include "inputManager.hpp"

void Texture::genTexture()
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    switch (nrChannels)
    {
    case 1:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        break;
    case 3:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        break;
    case 4:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        break;
    default:
        std::cerr << "Unsupported number of channels: " << nrChannels << std::endl;
        std::cerr << "Program will probably crash" << std::endl;
        break;
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

CubeMapPtr loadCubeMap(std::array<std::string, 6> faces_filenames)
{
    return std::make_shared<CubeMap>(faces_filenames);
}
CubeMapPtr loadCubeMap(std::string filename)
{
    return std::make_shared<CubeMap>(filename);
}

std::array<FBOPtr, 8> fbos;

void initializeFBOs()
{
    for (size_t i = 0; i < fbos.size(); i++)
    {
        fbos[i] = std::make_shared<FBO>(EngineGlobals::windowSize.x, EngineGlobals::windowSize.y);
    }

    InputManager::addWindowSizeCallback([](GLFWwindow *window, i32 width, i32 height) {
        // might break everything or might not idk
        for (size_t i = 0; i < fbos.size(); i++)
        {
            fbos[i] = std::make_shared<FBO>(width, height);
        }
    });
}
