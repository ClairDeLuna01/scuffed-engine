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
    template <typename... Paths> Material(ShaderProgramPtr shader, Paths... paths) : shader(shader)
    {
        (addTexture(loadTexture(paths)), ...);
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
    }

    ShaderProgramPtr getShader() const
    {
        return shader;
    }
};

using MaterialPtr = std::shared_ptr<Material>;
