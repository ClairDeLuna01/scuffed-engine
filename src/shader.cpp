#include "shader.hpp"

Shader::~Shader()
{

    if (this->ID != SHADER_NULL)
    {

        glDeleteShader(this->ID);
    }
}

Shader::Shader(std::string filename)
{

    // get file extension for implicit function call
    std::string extension = getExtension(filename);

    if (extension.compare("vert") == 0)
    {
        Shader(filename, VERTEX);
    }
    else if (extension.compare("frag") == 0)
    {
        Shader(filename, FRAGMENT);
    }
    else
    {
        std::cerr << "Shader " << stripPath(filename)
                  << " doesn't have an explicit extension, please provide the shader type in the constructor.\n";
    }
}

void Shader::load(std::string filename)
{

    // get file extension for implicit function call
    std::string extension = getExtension(filename);

    if (extension.compare("vert") == 0)
    {
        load(filename, VERTEX);
    }
    else if (extension.compare("frag") == 0)
    {
        load(filename, FRAGMENT);
    }
    else
    {
        std::cerr << "Shader " << stripPath(filename)
                  << " doesn't have an explicit extension, please provide the shader type in the constructor.\n";
    }
}

Shader::Shader(std::string filename, u32 _type)
{

    this->shaderName = stripPath(filename);
    this->path = filename;

    // assign type
    switch (_type)
    {
    case VERTEX:
        this->type = GL_VERTEX_SHADER;
        break;
    case FRAGMENT:
        this->type = GL_FRAGMENT_SHADER;
        break;
    default:
        std::cerr << "Invalid type provided for shader " << this->shaderName << ", (expected VERTEX or FRAGMENT).\n";
        exit(EXIT_FAILURE);
    }

    this->ID = glCreateShader(this->type);
}

void Shader::load(std::string filename, u32 _type)
{

    this->shaderName = stripPath(filename);
    this->path = filename;

    // assign type
    switch (_type)
    {
    case VERTEX:
        this->type = GL_VERTEX_SHADER;
        break;
    case FRAGMENT:
        this->type = GL_FRAGMENT_SHADER;
        break;
    default:
        std::cerr << "Invalid type provided for shader " << this->shaderName << ", (expected VERTEX or FRAGMENT).\n";
        exit(EXIT_FAILURE);
    }

    this->ID = glCreateShader(this->type);
}

void Shader::compile()
{

    if (this->ID == SHADER_NULL)
    {
        std::cerr << "Can't compile non initialized shader.\n";
        exit(EXIT_FAILURE);
    }

    // read shader file
    const size_t readSize = 4096;
    std::ifstream file(this->path, std::ios::in);
    if (!file.is_open())
    {
        std::cerr << "Could not open file " << this->path << "\n";
        exit(EXIT_FAILURE);
    }

    std::string buf = std::string(readSize, '\0');
    while (file.read(&buf[0], readSize))
    {
        this->source.append(buf, 0, file.gcount());
    }
    this->source.append(buf, 0, file.gcount());

    // std::cout << "Shader Constructor: successfully read the content of " << this->shaderName << ".\n";

    // source shader from extracted content
    const GLchar *source = (const GLchar *)this->source.c_str();
    glShaderSource(this->ID, 1, &source, 0);

    // compile shader
    glCompileShader(this->ID);

    // check if shader compiled correctly
    glGetShaderiv(this->ID, GL_COMPILE_STATUS, (int *)&this->_isCompiled);
    if (this->_isCompiled == GL_FALSE)
    {

        // get info log length
        GLint maxLength = 0;
        glGetShaderiv(this->ID, GL_INFO_LOG_LENGTH, &maxLength);

        // get info log
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(this->ID, maxLength, &maxLength, &infoLog[0]);

        // delete shader
        this->_delete();

        // print info log
        std::cerr << "Error compiling shader " << this->shaderName << ", see info log for more details.\n";
        for (auto c : infoLog)
            std::cerr << c;
        std::cerr << "\n";

        // goodbye
        exit(EXIT_FAILURE);
    }

    // std::cout << "Successfully compiled shader " << this->shaderName << ".\n";
}

std::string Shader::getType()
{

    switch (this->type)
    {
    case VERTEX:
        return "VERTEX";
    case FRAGMENT:
        return "FRAGMENT";
    default:
        return "INVALID TYPE";
    }
}

void Shader::_delete()
{

    glDeleteShader(this->ID);
    this->ID = SHADER_NULL;
}

ShaderProgram::ShaderProgram(std::string vertPath, std::string fragPath)
{

    // load and compile shaders
    this->vert.load(vertPath);
    this->frag.load(fragPath);

    this->ID = glCreateProgram();

    // compile shaders
    link();
}

void ShaderProgram::load(std::string vertPath, std::string fragPath)
{

    // load and compile shaders
    this->vert.load(vertPath);
    this->frag.load(fragPath);

    this->ID = glCreateProgram();
}

void ShaderProgram::link()
{

    if (this->ID == PROGRAM_NULL)
    {
        std::cerr << "Can't link non initialized shader program.\n";
        exit(EXIT_FAILURE);
    }

    // compile shaders
    this->vert.compile();
    this->frag.compile();

    // attach shaders
    glAttachShader(this->ID, this->vert.getID());
    glAttachShader(this->ID, this->frag.getID());

    // link program
    glLinkProgram(this->ID);

    // check if program is linked
    glGetProgramiv(this->ID, GL_LINK_STATUS, (int *)&this->_isLinked);
    if (this->_isLinked == GL_FALSE)
    {

        // get info log length
        GLint maxLength = 0;
        glGetProgramiv(this->ID, GL_INFO_LOG_LENGTH, &maxLength);

        // get info log
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(this->ID, maxLength, &maxLength, &infoLog[0]);

        // delete program and shaders
        this->_delete();
        this->vert._delete();
        this->frag._delete();

        // print info log
        std::cerr << "Error linking program ID " << this->ID << ", see info log for more details.\n";
        for (auto c : infoLog)
            std::cerr << c;
        std::cerr << "\n";

        // goodbye
        exit(EXIT_FAILURE);
    }

    // detach shaders
    glDetachShader(this->ID, this->vert.getID());
    glDetachShader(this->ID, this->frag.getID());
    this->vert._delete();
    this->frag._delete();

    // std::cout << "Successfully linked program ID " << this->ID << ".\n";
}

void ShaderProgram::use()
{

    if (this->ID != PROGRAM_NULL && this->_isLinked == GL_TRUE)
    {

        glUseProgram(this->ID);
    }
    else
    {

        std::cerr << "Can't use non initalized/linked shader program.\n";
        exit(EXIT_FAILURE);
    }
}

void ShaderProgram::stop()
{

    glUseProgram(0);
}

ShaderProgram::~ShaderProgram()
{

    if (this->ID != PROGRAM_NULL)
    {

        glDeleteProgram(this->ID);
    }
}

void ShaderProgram::_delete()
{

    glDeleteProgram(this->ID);
    this->ID = PROGRAM_NULL;
}

void ShaderProgram::setUniform(i32 location, const mat4 &value)
{

    if (this->ID != PROGRAM_NULL && this->_isLinked == GL_TRUE)
    {
        use();
        glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    }
    else
    {

        std::cerr << "Can't set uniform for non initalized/linked shader program.\n";
        exit(EXIT_FAILURE);
    }
}

void ShaderProgram::setUniform(i32 location, const vec3 &value)
{
    if (this->ID != PROGRAM_NULL && this->_isLinked == GL_TRUE)
    {
        use();
        glUniform3fv(location, 1, &value[0]);
    }
    else
    {

        std::cerr << "Can't set uniform for non initalized/linked shader program.\n";
        exit(EXIT_FAILURE);
    }
}

void ShaderProgram::setUniform(i32 location, const vec2 &value)
{
    if (this->ID != PROGRAM_NULL && this->_isLinked == GL_TRUE)
    {
        use();
        glUniform2fv(location, 1, &value[0]);
    }
    else
    {

        std::cerr << "Can't set uniform for non initalized/linked shader program.\n";
        exit(EXIT_FAILURE);
    }
}

void ShaderProgram::setUniform(i32 location, const vec4 &value)
{
    if (this->ID != PROGRAM_NULL && this->_isLinked == GL_TRUE)
    {
        use();
        glUniform4fv(location, 1, &value[0]);
    }
    else
    {

        std::cerr << "Can't set uniform for non initalized/linked shader program.\n";
        exit(EXIT_FAILURE);
    }
}

void ShaderProgram::setUniform(i32 location, const f32 &value)
{
    if (this->ID != PROGRAM_NULL && this->_isLinked == GL_TRUE)
    {
        use();
        glUniform1f(location, value);
    }
    else
    {

        std::cerr << "Can't set uniform for non initalized/linked shader program.\n";
        exit(EXIT_FAILURE);
    }
}

void ShaderProgram::setUniform(i32 location, const i32 &value)
{
    if (this->ID != PROGRAM_NULL && this->_isLinked == GL_TRUE)
    {
        use();
        glUniform1i(location, value);
    }
    else
    {

        std::cerr << "Can't set uniform for non initalized/linked shader program.\n";
        exit(EXIT_FAILURE);
    }
}

void ShaderProgram::setUniform(i32 location, const u32 &value)
{
    if (this->ID != PROGRAM_NULL && this->_isLinked == GL_TRUE)
    {
        use();
        glUniform1ui(location, value);
    }
    else
    {
        std::cerr << "Can't set uniform for non initalized/linked shader program.\n";
        exit(EXIT_FAILURE);
    }
}

ShaderProgramPtr newShaderProgram(std::string vertPath, std::string fragPath)
{
    return std::make_shared<ShaderProgram>(vertPath, fragPath);
}

ShaderProgramPtr newShaderProgram()
{
    return std::make_shared<ShaderProgram>();
}
