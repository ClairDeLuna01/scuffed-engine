#pragma once
#include "stb_image.h"
#include "typedef.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <array>

class Texture
{
private:
    // image data
    i32 width, height, nrChannels;
    u8 *data;

    // opengl stuff
    GLuint textureID;

    void genTexture();

public:
    Texture(const char *path)
    {
        data = stbi_load(path, &width, &height, &nrChannels, 0);
        genTexture();
    }

    ~Texture()
    {
        stbi_image_free(data);

        glDeleteTextures(1, &textureID);
    }

    inline i32 getWidth()
    {
        return width;
    }

    inline i32 getHeight()
    {
        return height;
    }

    inline i32 getNrChannels()
    {
        return nrChannels;
    }

    inline u8 *getData()
    {
        return data;
    }

    inline GLuint getTextureID()
    {
        return textureID;
    }

    inline void bind()
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
};

using TexturePtr = std::shared_ptr<Texture>;

TexturePtr loadTexture(const char *path);