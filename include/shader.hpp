#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "typedef.hpp"
#include "utils.hpp"

#define SHADER_NULL 0xffffffff
#define PROGRAM_NULL 0xffffffff

// can add more types later
enum ShaderType
{
    VERTEX,
    FRAGMENT
};

class Shader
{

    friend class ShaderProgram;

private:
    u32 ID = SHADER_NULL;
    u32 type;
    std::string path;
    std::string source;
    std::string shaderName;
    u32 _isCompiled = GL_FALSE;

    void _delete();

public:
    Shader(){};
    Shader(std::string filename);
    Shader(std::string filename, u32 _type);
    ~Shader();

    void load(std::string filename);
    void load(std::string filename, u32 _type);
    void compile();

    u32 getID() { return ID; };
    u32 getRawType() { return type; };
    std::string getType();
    std::string getShaderName() { return shaderName; };
    u32 isCompiled() { return _isCompiled; };
};

class ShaderProgram
{

private:
    u32 ID = PROGRAM_NULL;
    Shader vert;
    Shader frag;
    u32 _isLinked = GL_FALSE;

    void _delete();

public:
    ShaderProgram(){};
    ShaderProgram(std::string vertPath, std::string fragPath);
    ~ShaderProgram();

    void load(std::string vertPath, std::string fragPath);
    void link();
    void use();
    void stop();

    u32 getID() { return ID; };
    u32 getVertID() { return vert.getID(); };
    u32 getFragID() { return frag.getID(); };
    u32 isLinked() { return _isLinked; };
};

ShaderProgramPtr newShaderProgram();

ShaderProgramPtr newShaderProgram(std::string vertPath, std::string fragPath);