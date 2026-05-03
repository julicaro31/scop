#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Mesh.hpp"

class Renderer
{
public:
    Renderer(int width, int height, const char *title);
    ~Renderer();
    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;

    void uploadMesh(const Mesh &mesh);
    void beginFrame();
    void draw() const;
    void endFrame();
    bool shouldClose() const;
    GLFWwindow *getWindow() const;

private:
    GLFWwindow *_window;
    GLuint _VAO;
    GLuint _VBO;
    GLuint _EBO;
    GLsizei _indexCount;
};

#endif
