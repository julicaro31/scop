#include "Shader.hpp"

// Constructor: reads, compiles, and links shaders from file paths
Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath)
{
    // OpenGL needs us to compile shaders at runtime (not at build time like C++).
    // This is because different GPUs have different instruction sets, so the GPU driver compiles GLSL into GPU-specific machine code.
    ShaderObject vertexShader(GL_VERTEX_SHADER);
    compileShader(vertexShader.get(), readFile(vertexPath), "VERTEX");

    ShaderObject fragmentShader(GL_FRAGMENT_SHADER);
    compileShader(fragmentShader.get(), readFile(fragmentPath), "FRAGMENT");

    // After linking, the compiled shaders are baked into the program. We don't need the individual shader objects anymore.
    programID = linkProgram(vertexShader.get(), fragmentShader.get());
}

Shader::~Shader()
{
    glDeleteProgram(programID);
}

// USE (activate this shader)
// OpenGL is a state machine. Calling glUseProgram tells OpenGL:
// "From now on, use THIS shader program for all drawing commands."
// Call this before glDrawArrays / glDrawElements.
void Shader::use() const
{
    glUseProgram(programID);
}

void Shader::setMat4(const std::string &name, const Mat4 &mat) const
{
    glUniformMatrix4fv(
        glGetUniformLocation(programID, name.c_str()),
        1,
        GL_FALSE, // column-major, no transpose needed
        mat.data());
}

// ============================================================================
// UNIFORM SETTERS
// Uniforms are variables you send from C++ to the shader.
// They stay the same for every vertex/pixel in a single draw call.
// Example uses: transformation matrices, colors, time, light positions.
//
// How it works:
//   1. glGetUniformLocation finds WHERE the uniform lives in the program
//      (returns -1 if the name doesn't exist or got optimized away)
//   2. glUniform* sends the actual value to that location
// ============================================================================
void Shader::setBool(const std::string &name, bool value) const
{
    // GLSL doesn't have a bool uniform type, so we pass it as an int (0 or 1)
    glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

std::string Shader::readFile(const std::string &path) const
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cerr << "SHADER ERROR: Could not open file: " << path << std::endl;
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    std::stringstream stream;
    stream << file.rdbuf();
    file.close();

    return stream.str();
}

GLuint Shader::compileShader(GLuint shader, const std::string &source, std::string typeName) const
{
    const char *sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, NULL);

    glCompileShader(shader);

    checkCompileErrors(shader, typeName);

    return shader;
}

GLuint Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) const
{
    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    try
    {
        checkLinkErrors(program);
    }
    catch (...)
    {
        glDeleteProgram(program);
        throw;
    }

    return program;
}

void Shader::checkCompileErrors(GLuint shader, const std::string &type) const
{
    GLint success;
    char infoLog[1024];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cerr << "SHADER COMPILATION ERROR (" << type << "):\n"
                  << infoLog
                  << "\n---------------------------------------------------"
                  << std::endl;
        throw std::runtime_error("Shader compilation failed: " + type);
    }
}

// Common link errors: mismatched in/out variables between vertex and fragment.
void Shader::checkLinkErrors(GLuint program) const
{
    GLint success;
    char infoLog[1024];

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success)
    {
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        std::cerr << "SHADER LINK ERROR:\n"
                  << infoLog
                  << "\n---------------------------------------------------"
                  << std::endl;
        throw std::runtime_error("Shader program linking failed");
    }
}
