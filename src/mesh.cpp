#include "mesh.hpp"
#include "assetLoader.hpp"

#include <sstream>
#include <fstream>
#include <algorithm>

#include <cstring>

Mesh::~Mesh()
{
    for (auto &vbo : vbos)
    {
        vbo.deleteBuffer();
    }
    ebo->deleteBuffer();
    glDeleteVertexArrays(1, &vaoID);
}

void Mesh::addVBO(VertexBufferObject &vbo)
{
    vbo.genBuffer();
    vbos.push_back(vbo);
}

void Mesh::setEBO(EBOptr &_ebo)
{
    ebo = std::move(_ebo);
}

void Mesh::bind()
{
    shader->use();
    glBindVertexArray(vaoID);
    for (auto &vbo : vbos)
    {
        vbo.bind();
    }
    ebo->bind();

    for (size_t i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        textures[i]->bind();

        setUniform(UNIFORM_LOCATIONS::TEXTURE0 + i, (i32)i);
    }
}

void Mesh::draw()
{
    bind();
    ebo->draw();
    unbind();
}

void Mesh::unbind()
{
    shader->stop();
    ebo->unbind();
    for (auto &vbo : vbos)
    {
        vbo.unbind();
    }
    glBindVertexArray(0);
}

MeshPtr Mesh::addTexture(TexturePtr &texture)
{
    textures.push_back(texture);
    return shared_from_this();
}

MeshPtr Mesh::addTexture(std::string filename)
{
    textures.push_back(loadTexture(filename.c_str()));
    return shared_from_this();
}

template <typename T, std::enable_if_t<
                          std::is_same<T, mat4>::value ||
                              std::is_same<T, vec3>::value ||
                              std::is_same<T, vec4>::value ||
                              std::is_same<T, f32>::value ||
                              std::is_same<T, i32>::value ||
                              std::is_same<T, u32>::value,
                          bool>>
MeshPtr Mesh::setUniform(u32 location, const T &value)
{
    shader->setUniform(location, value);
    return shared_from_this();
}

template <typename T, std::enable_if_t<
                          std::is_same<T, f64>::value, bool>>
MeshPtr Mesh::setUniform(u32 location, const T &value)
{
    shader->setUniform(location, (f32)value);
    return shared_from_this();
}

template <typename T, std::enable_if_t<
                          std::is_same<T, i64>::value ||
                              std::is_same<T, i16>::value ||
                              std::is_same<T, i8>::value,
                          bool>>
MeshPtr Mesh::setUniform(u32 location, const T &value)
{
    shader->setUniform(location, (i32)value);
    return shared_from_this();
}

template <typename T, std::enable_if_t<
                          std::is_same<T, u64>::value ||
                              std::is_same<T, u16>::value ||
                              std::is_same<T, u8>::value,
                          bool>>
MeshPtr Mesh::setUniform(u32 location, const T &value)
{
    shader->setUniform(location, (u32)value);
    return shared_from_this();
}

void Mesh::FromFile(const char *path, std::vector<uivec3> &indices, std::vector<vec3> &vertices, std::vector<vec3> &normals, std::vector<vec2> &uvs)
{
    loadMesh(path, indices, vertices, normals, uvs);
}

MeshPtr loadMesh(ShaderProgramPtr shader, std::string filename)
{
    return std::make_shared<Mesh>(shader, filename);
}

SkyboxPtr loadSkybox(ShaderProgramPtr shader, CubeMapPtr cubeMap)
{
    return std::make_shared<Skybox>(shader, cubeMap);
}

template MeshPtr Mesh::setUniform(u32 location, const mat4 &value);
template MeshPtr Mesh::setUniform(u32 location, const vec3 &value);
template MeshPtr Mesh::setUniform(u32 location, const vec4 &value);
template MeshPtr Mesh::setUniform(u32 location, const f32 &value);
template MeshPtr Mesh::setUniform(u32 location, const f64 &value);
template MeshPtr Mesh::setUniform(u32 location, const i64 &value);
template MeshPtr Mesh::setUniform(u32 location, const i32 &value);
template MeshPtr Mesh::setUniform(u32 location, const i16 &value);
template MeshPtr Mesh::setUniform(u32 location, const i8 &value);
template MeshPtr Mesh::setUniform(u32 location, const u64 &value);
template MeshPtr Mesh::setUniform(u32 location, const u32 &value);
template MeshPtr Mesh::setUniform(u32 location, const u16 &value);
template MeshPtr Mesh::setUniform(u32 location, const u8 &value);
