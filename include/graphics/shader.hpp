#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include <string>

class Shader
{
public:
    GLuint programID;

    Shader(const std::string &vertexPath, const std::string &fragmentPath);

    ~Shader();

    void use() const;

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;

private:
    std::string readFile(const std::string &path) const;

    GLuint compileShader(GLenum type, const std::string &source) const;

    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader) const;

    void checkCompileErrors(GLuint shader, const std::string &type) const;
    void checkLinkErrors(GLuint program) const;
};

#endif
