#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "shader.hpp"
#include "typedef.hpp"
#include "transform3D.hpp"
#include "texture.hpp"

#include "globals.hpp"

using namespace EngineGlobals;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace glm;

class ElementBufferObject
{
private:
    GLenum mode = GL_TRIANGLES;
    GLenum usage = GL_STATIC_DRAW;

    void *data;
    u64 dataLength;

    GLenum type = GL_UNSIGNED_INT;
    u32 elementSize = sizeof(unsigned int);

    u64 offset = 0;

    GLuint bufferID;

    inline void genBuffer()
    {
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataLength * elementSize, data, GL_STATIC_DRAW);
    }

public:
    ElementBufferObject(void *_data, u64 _dataLength) : data(_data), dataLength(_dataLength)
    {
        genBuffer();
    }
    ElementBufferObject(void *_data, u64 _dataLength, GLenum _type, u32 _elementSize, GLenum _usage, u64 _offset) : usage(_usage), data(_data), dataLength(_dataLength), type(_type), elementSize(_elementSize), offset(_offset)
    {
        genBuffer();
    }

    inline void deleteBuffer()
    {
        glDeleteBuffers(1, &bufferID);
    }

    inline void bind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
    }

    inline void unbind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    inline void draw() const
    {
        glDrawElements(mode, dataLength, type, (void *)offset);
    }

    void update(void *_data)
    {
        data = _data;
    }
};

class VertexBufferObject
{
private:
    GLint elementCount;
    u64 elementSize;
    GLuint location;
    GLenum type;
    GLboolean normalized = GL_FALSE;
    GLenum usage = GL_STATIC_DRAW;
    GLsizei stride = 0;

    void *data;
    u64 dataLength = 0;
    u64 offset = 0;

    GLuint bufferID;

public:
    VertexBufferObject(GLint _elementCount, u64 _elementSize, GLuint _location, GLenum _type, void *_data, u64 _dataLength) : elementCount(_elementCount), elementSize(_elementSize), location(_location), type(_type), data(_data), dataLength(_dataLength)
    {
    }

    inline void deleteBuffer()
    {
        glDeleteBuffers(1, &bufferID);
    }

    inline void genBuffer()
    {
        // go look there https://stackoverflow.com/questions/21652546/what-is-the-role-of-glbindvertexarrays-vs-glbindbuffer-and-what-is-their-relatio
        glGenBuffers(1, &bufferID);
        if (bufferID == 0)
        {
            std::cerr << "Failed to create Vertex Attrib Object.\n";
            exit(EXIT_FAILURE);
        }
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, dataLength * elementCount * elementSize, data, GL_STATIC_DRAW);
    }

    inline void bind()
    {
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glVertexAttribPointer(location, elementCount, type, normalized, stride, (void *)offset);
    }

    inline void unbind()
    {
        glDisableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void update(void *_data)
    {
        data = _data;

        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, dataLength * elementCount, data, GL_STATIC_DRAW);
    }
};

class Mesh
{
protected:
    std::shared_ptr<ShaderProgram> shader;

    std::vector<VertexBufferObject> vbos;
    EBOptr ebo = nullptr;

    GLuint vaoID;

    std::vector<TexturePtr> textures;

public:
    Mesh(std::shared_ptr<ShaderProgram> _shader) : shader(_shader)
    {
        glGenVertexArrays(1, &vaoID);
    }

    ~Mesh();

    void addVBO(VertexBufferObject &vbo);

    void setEBO(EBOptr &_ebo);

    virtual void bind();

    virtual void draw();

    virtual void unbind();

    template <typename T, std::enable_if_t<
                              std::is_same<T, mat4>::value ||
                                  std::is_same<T, vec3>::value ||
                                  std::is_same<T, vec4>::value ||
                                  std::is_same<T, f32>::value ||
                                  std::is_same<T, i32>::value ||
                                  std::is_same<T, u32>::value,
                              bool> = true>
    void setUniform(u32 location, const T &value);

    template <typename T, std::enable_if_t<
                              std::is_same<T, f64>::value, bool> = true>
    void setUniform(u32 location, const T &value);

    template <typename T, std::enable_if_t<
                              std::is_same<T, i64>::value ||
                                  std::is_same<T, i16>::value ||
                                  std::is_same<T, i8>::value,
                              bool> = true>
    void setUniform(u32 location, const T &value);

    template <typename T, std::enable_if_t<
                              std::is_same<T, u64>::value ||
                                  std::is_same<T, u16>::value ||
                                  std::is_same<T, u8>::value,
                              bool> = true>
    void setUniform(u32 location, const T &value);

    void addTexture(TexturePtr &texture);
};

class Mesh3D : public Mesh
{
private:
    Transform3D transform;

public:
    Mesh3D(std::shared_ptr<ShaderProgram> _shader) : Mesh(_shader) {}

    void setTransform(const Transform3D &t)
    {
        transform = t;
    }

    void bind() override
    {
        shader->use();
        shader->setUniform(1, transform.getModel());
        shader->setUniform(2, viewMatrix);
        shader->setUniform(3, projectionMatrix);

        Mesh::bind();
    };

    // temporary until we have a model loader
    static void Cube(std::vector<uivec3> &indices, std::vector<vec3> &vertices, std::vector<vec3> &normals);
    static void Sphere(std::vector<uivec3> &indices, std::vector<vec3> &vertices, std::vector<vec3> &normals, std::vector<vec2> &uvs, u32 sectorCount = 36, u32 stackCount = 18);
    static void FromOBJ(const char *path, std::vector<uivec3> &indices, std::vector<vec3> &vertices, std::vector<vec3> &normals, std::vector<vec2> &uvs);
};