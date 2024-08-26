#pragma once

#include <vector>

#include "shader.hpp"
#include "texture.hpp"
#include "typedef.hpp"

class Material
{
  private:
    ShaderProgramPtr shader;
    std::vector<TexturePtr> textures;

  public:
    Material(ShaderProgramPtr shader) : shader(shader)
    {
    }

    ~Material() = default;

    void addTexture(TexturePtr texture)
    {
        textures.push_back(texture);
    }
    void use() const
    {
        shader->use();
        for (size_t i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            textures[i]->bind();

            shader->setUniform(UNIFORM_LOCATIONS::TEXTURE0 + i, (i32)i);
        }
    }

    void stop() const
    {
        shader->stop();
        for (size_t i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            textures[i]->unbind();
        }
    }

    ShaderProgramPtr getShader() const
    {
        return shader;
    }

    u32 getTextureCount() const
    {
        return textures.size();
    }
};

using MaterialPtr = std::shared_ptr<::Material>;
