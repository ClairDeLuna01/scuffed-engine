#include "shader.hpp"
#include "texture.hpp"

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
        Shader(filename, ShaderType::VERTEX);
    }
    else if (extension.compare("frag") == 0)
    {
        Shader(filename, ShaderType::FRAGMENT);
    }
    else if (extension.compare("geom") == 0)
    {
        Shader(filename, ShaderType::GEOMETRY);
    }
    else
    {
        std::cerr << "Shader " << stripPath(filename)
                  << " doesn't have an explicit extension, please provide the shader type in the constructor.\n";
    }
}

ShaderPtr Shader::load(std::string filename)
{
    // get file extension for implicit function call
    std::string extension = getExtension(filename);

    if (extension.compare("vert") == 0)
    {
        return load(filename, ShaderType::VERTEX);
    }
    else if (extension.compare("frag") == 0)
    {
        return load(filename, ShaderType::FRAGMENT);
    }
    else if (extension.compare("geom") == 0)
    {
        return load(filename, ShaderType::GEOMETRY);
    }
    else
    {
        std::cerr << "Shader " << stripPath(filename)
                  << " doesn't have an explicit extension, please provide the shader type in the constructor.\n";
    }

    return nullptr;
}

Shader::Shader(std::string filename, ShaderType _type)
{
    this->type = _type;
    this->shaderName = stripPath(filename);
    this->path = filename;

    // assign type
    switch (_type)
    {
    case ShaderType::VERTEX:
        this->typeID = GL_VERTEX_SHADER;
        break;
    case ShaderType::FRAGMENT:
        this->typeID = GL_FRAGMENT_SHADER;
        break;
    case ShaderType::GEOMETRY:
        this->typeID = GL_GEOMETRY_SHADER;
        break;
    default:
        std::cerr << "Invalid type provided for shader " << this->shaderName
                  << ", (expected VERTEX or FRAGMENT or GEOMETRY).\n";
        exit(EXIT_FAILURE);
    }

    this->ID = glCreateShader(this->typeID);
}

ShaderPtr Shader::load(std::string filename, ShaderType _type)
{
    ShaderPtr shader = std::make_unique<Shader>(filename, _type);

    return shader;
}

std::string getFileContent(std::string filename)
{
    std::string output;
    std::ifstream file(filename, std::ios::in);
    if (!file.is_open())
    {
        std::cerr << "Could not open file " << filename << "\n";
        exit(EXIT_FAILURE);
    }

    // parse content of shader file for #include directives
    std::string line;
    while (std::getline(file, line))
    {
        // check if line is a #include directive
        if (line.length() > 10 && line[0] == '#' && *(u64 *)(line.data() + 1) == *(u64 *)("include "))
        {
            constexpr char includePath[] = "shader/";
            // extract filename from #include directive
            std::string includeFilename = includePath + line.substr(10, line.size() - 11);

            std::cout << "Including file " << includeFilename << "\n";

            output.append(getFileContent(includeFilename));
        }
        else
        {
            output.append(line + "\n");
        }
    }

    return output;
}

void Shader::compile()
{

    if (this->ID == SHADER_NULL)
    {
        std::cerr << "Can't compile non initialized shader.\n";
        exit(EXIT_FAILURE);
    }

    // read shader file
    // const size_t readSize = 4096;
    std::ifstream file(this->path, std::ios::in);
    if (!file.is_open())
    {
        std::cerr << "Could not open file " << this->path << "\n";
        exit(EXIT_FAILURE);
    }

    // std::string buf = std::string(readSize, '\0');
    // while (file.read(&buf[0], readSize))
    // {
    //     this->source.append(buf, 0, file.gcount());
    // }
    // this->source.append(buf, 0, file.gcount());

    // parse content of shader file for #include directives
    this->source = getFileContent(this->path);

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
    case ShaderType::VERTEX:
        return "VERTEX";
    case ShaderType::FRAGMENT:
        return "FRAGMENT";
    case ShaderType::GEOMETRY:
        return "GEOMETRY";
    default:
        return "INVALID TYPE";
    }
}

void Shader::_delete()
{

    glDeleteShader(this->ID);
    this->ID = SHADER_NULL;
}

ShaderProgram::ShaderProgram(std::string vertPath, std::string fragPath, bool hasAccesstoFramebuffers)
{
    // load shaders
    this->vert = Shader::load(vertPath, ShaderType::VERTEX);
    this->frag = Shader::load(fragPath, ShaderType::FRAGMENT);

    this->hasAccesstoFramebuffers = hasAccesstoFramebuffers;

    this->ID = glCreateProgram();

    // compile shaders
    link();

    if (this->hasAccesstoFramebuffers)
    {
        for (int i = 0; i < FBO_N; i++)
        {
            setUniform(UNIFORM_LOCATIONS::FRAMEBUFFER0 + i, 16 + i);
        }
    }
}

ShaderProgram::ShaderProgram(std::string vertPath, std::string fragPath, std::string geomPath,
                             bool hasAccesstoFramebuffers)
{
    // load shaders
    this->vert = Shader::load(vertPath, ShaderType::VERTEX);
    this->frag = Shader::load(fragPath, ShaderType::FRAGMENT);
    this->geom = Shader::load(geomPath, ShaderType::GEOMETRY);

    this->hasAccesstoFramebuffers = hasAccesstoFramebuffers;

    this->ID = glCreateProgram();

    // compile shaders
    link();

    if (this->hasAccesstoFramebuffers)
    {
        for (int i = 0; i < FBO_N; i++)
        {
            setUniform(UNIFORM_LOCATIONS::FRAMEBUFFER0 + i, 16 + i);
        }
    }
}

void ShaderProgram::link()
{

    if (this->ID == PROGRAM_NULL)
    {
        std::cerr << "Can't link non initialized shader program.\n";
        exit(EXIT_FAILURE);
    }

    // compile shaders
    this->vert->compile();
    this->frag->compile();
    if (this->geom)
    {
        this->geom->compile();
    }

    // attach shaders
    glAttachShader(this->ID, this->vert->getID());
    glAttachShader(this->ID, this->frag->getID());
    if (this->geom)
    {
        glAttachShader(this->ID, this->geom->getID());
    }

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
        this->vert->_delete();
        this->frag->_delete();
        if (this->geom)
        {
            this->geom->_delete();
        }

        // print info log
        std::cerr << "Error linking program ID " << this->ID << ", see info log for more details.\n";
        for (auto c : infoLog)
            std::cerr << c;
        std::cerr << "\n";

        // goodbye
        exit(EXIT_FAILURE);
    }

    // detach shaders
    glDetachShader(this->ID, this->vert->getID());
    glDetachShader(this->ID, this->frag->getID());
    if (this->geom)
    {
        glDetachShader(this->ID, this->geom->getID());
    }
    this->vert->_delete();
    this->frag->_delete();
    if (this->geom)
    {
        this->geom->_delete();
    }

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

ShaderProgramPtr newShaderProgram(std::string vertPath, std::string fragPath, bool hasAccesstoFramebuffers)
{
    return std::make_shared<ShaderProgram>(vertPath, fragPath, hasAccesstoFramebuffers);
}

ShaderProgramPtr newShaderProgram()
{
    return std::make_shared<ShaderProgram>();
}
