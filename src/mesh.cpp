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

        setUniform(500 + i, (i32)i);
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

void Mesh::addTexture(TexturePtr &texture)
{
    textures.push_back(texture);
}

template <typename T, std::enable_if_t<
                          std::is_same<T, mat4>::value ||
                              std::is_same<T, vec3>::value ||
                              std::is_same<T, vec4>::value ||
                              std::is_same<T, f32>::value ||
                              std::is_same<T, i32>::value ||
                              std::is_same<T, u32>::value,
                          bool>>
void Mesh::setUniform(u32 location, const T &value)
{
    shader->setUniform(location, value);
}

template <typename T, std::enable_if_t<
                          std::is_same<T, f64>::value, bool>>
void Mesh::setUniform(u32 location, const T &value)
{
    shader->setUniform(location, (f32)value);
}

template <typename T, std::enable_if_t<
                          std::is_same<T, i64>::value ||
                              std::is_same<T, i16>::value ||
                              std::is_same<T, i8>::value,
                          bool>>
void Mesh::setUniform(u32 location, const T &value)
{
    shader->setUniform(location, (i32)value);
}

template <typename T, std::enable_if_t<
                          std::is_same<T, u64>::value ||
                              std::is_same<T, u16>::value ||
                              std::is_same<T, u8>::value,
                          bool>>
void Mesh::setUniform(u32 location, const T &value)
{
    shader->setUniform(location, (u32)value);
}

void Mesh3D::Sphere(std::vector<uivec3> &indices, std::vector<vec3> &vertices, std::vector<vec3> &normals, std::vector<vec2> &uvs, u32 sectorCount, u32 stackCount)
{
    f32 x, y, z, xy;
    f32 nx, ny, nz, lengthInv = 1.0f / 1.0f;
    f32 s, t;

    f32 sectorStep = 2 * M_PI / sectorCount;
    f32 stackStep = M_PI / stackCount;
    f32 sectorAngle, stackAngle;

    for (u32 i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = 1.0f * cosf(stackAngle);
        z = 1.0f * sinf(stackAngle);

        for (u32 j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            vertices.push_back(vec3(x, y, z));

            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            normals.push_back(vec3(nx, ny, nz));

            // texture coordinates
            s = (f32)j / sectorCount;
            t = (f32)i / stackCount;

            uvs.push_back(vec2(s, t));
        }
    }

    u32 k1, k2;
    for (u32 i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (u32 j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(uivec3(k1, k2, k1 + 1));
            }

            if (i != (stackCount - 1))
            {
                indices.push_back(uivec3(k1 + 1, k2, k2 + 1));
            }
        }
    }
}

void Mesh3D::Cube(std::vector<uivec3> &indices, std::vector<vec3> &vertices, std::vector<vec3> &normals)
{
    // Create a unit cube
    vertices = {
        vec3(-0.5f, -0.5f, -0.5f),
        vec3(0.5f, -0.5f, -0.5f),
        vec3(0.5f, 0.5f, -0.5f),
        vec3(-0.5f, 0.5f, -0.5f),
        vec3(-0.5f, -0.5f, 0.5f),
        vec3(0.5f, -0.5f, 0.5f),
        vec3(0.5f, 0.5f, 0.5f),
        vec3(-0.5f, 0.5f, 0.5f)};

    indices = {
        uivec3(0, 1, 2),
        uivec3(0, 2, 3),
        uivec3(1, 5, 6),
        uivec3(1, 6, 2),
        uivec3(5, 4, 7),
        uivec3(5, 7, 6),
        uivec3(4, 0, 3),
        uivec3(4, 3, 7),
        uivec3(3, 2, 6),
        uivec3(3, 6, 7),
        uivec3(4, 5, 1),
        uivec3(4, 1, 0)};

    normals.clear();
    normals.resize(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++)
    {
        normals[i] = normalize(vertices[i]);
    }
}

void Mesh3D::FromFile(const char *path, std::vector<uivec3> &indices, std::vector<vec3> &vertices, std::vector<vec3> &normals, std::vector<vec2> &uvs)
{
    loadMesh(path, indices, vertices, normals, uvs);

    std::cout << "indices: " << indices.size() << std::endl;
    std::cout << "vertices: " << vertices.size() << std::endl;
    std::cout << "normals: " << normals.size() << std::endl;
    std::cout << "uvs: " << uvs.size() << std::endl;
}

template void
Mesh::setUniform(u32 location, const mat4 &value);
template void Mesh::setUniform(u32 location, const vec3 &value);
template void Mesh::setUniform(u32 location, const vec4 &value);
template void Mesh::setUniform(u32 location, const f32 &value);
template void Mesh::setUniform(u32 location, const f64 &value);
template void Mesh::setUniform(u32 location, const i64 &value);
template void Mesh::setUniform(u32 location, const i32 &value);
template void Mesh::setUniform(u32 location, const i16 &value);
template void Mesh::setUniform(u32 location, const i8 &value);
template void Mesh::setUniform(u32 location, const u64 &value);
template void Mesh::setUniform(u32 location, const u32 &value);
template void Mesh::setUniform(u32 location, const u16 &value);
template void Mesh::setUniform(u32 location, const u8 &value);
