#include "mesh.hpp"
#include "assetLoader.hpp"
#include "assetManager.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

#include <cstring>

std::shared_ptr<DirectionalLight> getSun()
{
    // Transform3D sunTransform;
    // sunTransform.setPosition(vec3(0.0f, 50.0f, 0.0f));
    // sunTransform.setRotation(vec3(0.0f, 0.0f, 0.0f));
    vec3 sunDir = vec3(0.0f, -1.0f, 0.0f);
    vec3 sunColor = vec3(1.0f, 1.0f, 1.0f);
    f32 sunIntensity = 0.75f;
    static auto sun = std::make_shared<DirectionalLight>(DirectionalLight{sunDir, sunColor, sunIntensity});
    return sun;
}

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
    material->use();
    glBindVertexArray(vaoID);
    for (auto &vbo : vbos)
    {
        vbo.bind();
    }
    ebo->bind();
}

void Mesh::draw()
{
    bind();
    ebo->draw();
    unbind();
}

void Mesh::unbind()
{
    material->stop();
    ebo->unbind();
    for (auto &vbo : vbos)
    {
        vbo.unbind();
    }
    glBindVertexArray(0);
}

MeshPtr Mesh::addTexture(TexturePtr &texture)
{
    material->addTexture(texture);
    return shared_from_this();
}

MeshPtr Mesh::addTexture(std::string filename)
{
    material->addTexture(AssetManager::getInstance().loadTexture(filename.c_str()));
    return shared_from_this();
}

template <typename T, std::enable_if_t<std::is_same<T, mat4>::value || std::is_same<T, vec3>::value ||
                                           std::is_same<T, vec4>::value || std::is_same<T, f32>::value ||
                                           std::is_same<T, i32>::value || std::is_same<T, u32>::value,
                                       bool>>
MeshPtr Mesh::setUniform(u32 location, const T &value)
{
    material->getShader()->setUniform(location, value);
    return shared_from_this();
}

template <typename T, std::enable_if_t<std::is_same<T, f64>::value, bool>>
MeshPtr Mesh::setUniform(u32 location, const T &value)
{
    material->getShader()->setUniform(location, (f32)value);
    return shared_from_this();
}

template <
    typename T,
    std::enable_if_t<std::is_same<T, i64>::value || std::is_same<T, i16>::value || std::is_same<T, i8>::value, bool>>
MeshPtr Mesh::setUniform(u32 location, const T &value)
{
    material->getShader()->setUniform(location, (i32)value);
    return shared_from_this();
}

template <
    typename T,
    std::enable_if_t<std::is_same<T, u64>::value || std::is_same<T, u16>::value || std::is_same<T, u8>::value, bool>>
MeshPtr Mesh::setUniform(u32 location, const T &value)
{
    material->getShader()->setUniform(location, (u32)value);
    return shared_from_this();
}

void Mesh::FromFile(const char *path, std::vector<uivec3> &indices, std::vector<vec3> &vertices,
                    std::vector<vec3> &normals, std::vector<vec2> &uvs)
{
    loadMesh(path, indices, vertices, normals, uvs);
}

MeshPtr loadMesh(MaterialPtr mat, std::string filename, RenderLayerPtr layer)
{
    return std::make_shared<Mesh>(mat, filename, layer);
}

SkyboxPtr loadSkybox(MaterialPtr mat, CubeMapPtr cubeMap)
{
    return std::make_shared<Skybox>(mat, cubeMap);
}

bool Mesh::meshIntersect(Ray r, vec3 &intersectionPoint, vec3 &normal) const
{
    for (size_t i = 0; i < indices.size(); i++)
    {
        vec3 v0 = vertices[indices[i].x];
        vec3 v1 = vertices[indices[i].y];
        vec3 v2 = vertices[indices[i].z];

        vec3 e1 = v1 - v0;
        vec3 e2 = v2 - v0;

        vec3 h = cross(r.direction, e2);
        f32 a = dot(e1, h);

        if (a > -0.00001 && a < 0.00001)
            continue;

        f32 f = 1.0f / a;
        vec3 s = r.origin - v0;
        f32 u = f * dot(s, h);

        if (u < 0.0 || u > 1.0)
            continue;

        vec3 q = cross(s, e1);
        f32 v = f * dot(r.direction, q);

        if (v < 0.0 || u + v > 1.0)
            continue;

        f32 t = f * dot(e2, q);

        if (t > 0.00001)
        {
            intersectionPoint = r.origin + r.direction * t;
            normal = normalize(cross(e1, e2));
            return true;
        }
    }

    return false;
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
