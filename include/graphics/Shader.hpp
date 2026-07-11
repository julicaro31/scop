#ifndef SHADER_HPP
#define SHADER_HPP

#include "Mat4.hpp"
#include "ShaderObject.hpp"
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

class Shader
{
public:
    GLuint programID;

    Shader(const std::string &vertexPath, const std::string &fragmentPath);

    ~Shader();

    void use() const;

    void setMat4(const std::string &name, const Mat4 &mat) const;

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;

private:
    std::string readFile(const std::string &path) const;

    GLuint compileShader(GLuint shader, const std::string &source, std::string typeName) const;

    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader) const;

    void checkCompileErrors(GLuint shader, const std::string &type) const;
    void checkLinkErrors(GLuint program) const;
};

#endif
