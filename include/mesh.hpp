#pragma once

#include <memory>

#include "shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace glm;

class VertexBufferObject
{
private:
    GLint elementSize;
    GLuint location;
    GLenum type;
    GLboolean normalized = GL_FALSE;
    GLenum usage = GL_STATIC_DRAW;
    GLsizei stride = 0;

    void *data;
    u64 dataLength = 0;
    u32 offset = 0;

    GLuint bufferID;

    inline void genBuffer()
    {
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, dataLength * elementSize, data, GL_STATIC_DRAW);
    }

public:
    VertexBufferObject(GLint _elementSize, GLuint _location, GLenum _type, void *_data, u64 _dataLength) : elementSize(_elementSize), location(_location), type(_type), data(_data), dataLength(_dataLength)
    {
        genBuffer();
    }
    VertexBufferObject(GLint _elementSize, GLuint _location, GLenum _type, GLboolean _normalized, GLenum _usage, GLsizei _stride, void *_data, u64 _dataLength) : elementSize(_elementSize), location(_location), type(_type), normalized(_normalized), usage(_usage), stride(_stride), data(_data), dataLength(_dataLength)
    {
        genBuffer();
    }

    ~VertexBufferObject()
    {
        glDeleteBuffers(1, &bufferID);
    }

    inline void bind()
    {
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glVertexAttribPointer(location, elementSize, type, normalized, stride, data);
    }

    inline void unbind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(location);
    }

    void update(void *_data)
    {
        data = _data;
    }
};

class VertexArrayObject
{
private:
    GLenum mode = GL_TRIANGLES;
    GLenum usage = GL_STATIC_DRAW;

    void *data;
    u64 dataLength;

    GLenum type = GL_UNSIGNED_INT;
    u32 elementSize = sizeof(unsigned int);

    u32 offset = 0;

    GLuint arrayID;

    inline void genArray()
    {
        glGenVertexArrays(1, &arrayID);
        if (arrayID == 0)
        {
            std::cerr << "Failed to create Vertex Array Object.\n";
            exit(EXIT_FAILURE);
        }
        else
        {
            std::cout << "Created VAO with ID: " << arrayID << std::endl;
        }
        glBindVertexArray(arrayID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataLength * elementSize, data, GL_STATIC_DRAW);
    }

public:
    VertexArrayObject(void *_data, u64 _dataLength) : data(_data), dataLength(_dataLength)
    {
        genArray();
    }

    VertexArrayObject(void *_data, u64 _dataLength, GLenum _type, u32 _elementSize, GLenum _usage, u32 _offset) : data(_data), dataLength(_dataLength), type(_type), elementSize(_elementSize), usage(_usage), offset(_offset)
    {
        genArray();
    }

    ~VertexArrayObject()
    {
        glDeleteVertexArrays(1, &arrayID);
    }

    inline void bind()
    {
        glBindVertexArray(arrayID);
        glDrawElements(mode, dataLength, type, (void *)offset);
    }

    inline void unbind()
    {
        glBindVertexArray(0);
    }

    void update(void *_data)
    {
        data = _data;
    }
};

class Mesh
{
private:
    std::shared_ptr<ShaderProgram> shader;

    std::vector<VertexBufferObject> vbos;
    std::unique_ptr<VertexArrayObject> vao;

    GLuint vertexAttributeObject;

public:
    Mesh(std::shared_ptr<ShaderProgram> _shader) : shader(_shader)
    {
        glGenVertexArrays(1, &vertexAttributeObject);
    }

    ~Mesh()
    {
        glDeleteVertexArrays(1, &vertexAttributeObject);
    }

    void addVBO(VertexBufferObject vbo)
    {
        vbos.push_back(vbo);
    }

    inline void setVAO(std::unique_ptr<VertexArrayObject> _vao)
    {
        vao = std::move(_vao);
    }

    void bind()
    {
        shader->use();
        glBindVertexArray(vertexAttributeObject);
        for (auto &vbo : vbos)
        {
            vbo.bind();
        }

        vao->bind();
    }

    void unbind()
    {
        shader->stop();
        for (auto &vbo : vbos)
        {
            vbo.unbind();
        }
        vao->unbind();
        glBindVertexArray(0);
    }
};