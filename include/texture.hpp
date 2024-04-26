#pragma once
#include "stb_image.h"
#include "typedef.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <string>

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
        if (!data)
        {
            std::cerr << "Failed to load texture at path: " << path << std::endl;
            return;
        }
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

class Image
{
  private:
    i32 width, height, nrChannels;
    u8 *data;

  public:
    Image(const char *path)
    {
        data = stbi_load(path, &width, &height, &nrChannels, 0);
    }

    Image(u8 *_data, i32 _width, i32 _height, i32 _nrChannels)
        : width(_width), height(_height), nrChannels(_nrChannels), data(_data)
    {
    }

    ~Image()
    {
        stbi_image_free(data);
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

    std::unique_ptr<Image> slice(i32 x, i32 y, i32 w, i32 h)
    {
        if (x < 0 || y < 0 || x + w > width || y + h > height)
        {
            return nullptr;
        }

        u8 *newData = new u8[w * h * nrChannels];
        for (i32 i = 0; i < h; i++)
        {
            for (i32 j = 0; j < w; j++)
            {
                for (i32 k = 0; k < nrChannels; k++)
                {
                    newData[(i * w + j) * nrChannels + k] = data[((i + y) * width + (j + x)) * nrChannels + k];
                }
            }
        }

        return std::make_unique<Image>(newData, w, h, nrChannels);
    }

    std::unique_ptr<Image> slice(f32 x, f32 y, f32 w, f32 h)
    {
        return slice((i32)(x * width), (i32)(y * height), (i32)(w * width), (i32)(h * height));
    }
};

class CubeMap
{
  private:
    std::array<u8 *, 6> faces;
    std::unique_ptr<Image> image;
    GLuint textureID;

    enum LoadType
    {
        MULTIPLE_FILES,
        SINGLE_FILE
    } loadType;

  public:
    CubeMap(std::array<std::string, 6> faces_filenames)
    {
        loadType = LoadType::MULTIPLE_FILES;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        for (size_t i = 0; i < faces_filenames.size(); i++)
        {
            i32 width, height, nrChannels;
            faces[i] = stbi_load(faces_filenames[i].c_str(), &width, &height, &nrChannels, 0);
            if (faces[i])
            {
                if (nrChannels == 3)
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB,
                                 GL_UNSIGNED_BYTE, faces[i]);
                else if (nrChannels == 4)
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA,
                                 GL_UNSIGNED_BYTE, faces[i]);
                else
                {
                    std::cerr << "Unsupported number of channels: " << nrChannels << std::endl;
                    std::cerr << "Program will probably crash" << std::endl;
                }
            }
            else
            {
                std::cerr << "Cubemap texture failed to load at path: " << faces_filenames[i] << std::endl;
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    CubeMap(std::string filename)
    {
        loadType = LoadType::SINGLE_FILE;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        image = std::make_unique<Image>(filename.c_str());
        i32 width = image->getWidth() / 4;
        i32 height = image->getHeight() / 3;
        i32 nrChannels = image->getNrChannels();

        for (size_t i = 0; i < 6; i++)
        {
            faces[i] = new u8[width * height * nrChannels];
        }

        std::unique_ptr<Image> front = image->slice(width, height, width, height);
        std::unique_ptr<Image> left = image->slice(0, height, width, height);
        std::unique_ptr<Image> right = image->slice(2 * width, height, width, height);
        std::unique_ptr<Image> back = image->slice(3 * width, height, width, height);
        std::unique_ptr<Image> top = image->slice(width, 0, width, height);
        std::unique_ptr<Image> bottom = image->slice(width, 2 * height, width, height);

        for (i32 i = 0; i < width * height * nrChannels; i++)
        {
            faces[0][i] = front->getData()[i];
            faces[1][i] = back->getData()[i];
            faces[2][i] = top->getData()[i];
            faces[3][i] = bottom->getData()[i];
            faces[4][i] = right->getData()[i];
            faces[5][i] = left->getData()[i];
        }

        for (size_t i = 0; i < 6; i++)
        {
            if (nrChannels == 3)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                             faces[i]);
            else if (nrChannels == 4)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA,
                             GL_UNSIGNED_BYTE, faces[i]);
            else
            {
                std::cerr << "Unsupported number of channels: " << nrChannels << std::endl;
                std::cerr << "Program will probably crash" << std::endl;
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    ~CubeMap()
    {
        glDeleteTextures(1, &textureID);

        if (loadType == LoadType::SINGLE_FILE)
        {
            for (size_t i = 0; i < 6; i++)
            {
                delete[] faces[i];
            }
        }
    }

    inline GLuint getTextureID()
    {
        return textureID;
    }

    inline void bind()
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    }
};

using CubeMapPtr = std::shared_ptr<CubeMap>;
CubeMapPtr loadCubeMap(std::array<std::string, 6> faces_filenames);
CubeMapPtr loadCubeMap(std::string filename);
