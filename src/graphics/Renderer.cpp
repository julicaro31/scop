#include "Renderer.hpp"
#include <iostream>
#include <stdexcept>

Renderer::Renderer(int width, int height, const char *title)
    : _window(nullptr), _VAO(0), _VBO(0), _vertexCount(0)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!_window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glEnable(GL_DEPTH_TEST);
}

Renderer::~Renderer()
{
    if (_VAO)
        glDeleteVertexArrays(1, &_VAO);
    if (_VBO)
        glDeleteBuffers(1, &_VBO);
    glfwTerminate();
}

void Renderer::uploadMesh(const Mesh &mesh)
{
    std::vector<float> data = mesh.expandedColoredVertices();
    _vertexCount = static_cast<GLsizei>(data.size() / 6); // 6 floats per vertex

    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);

    glBindVertexArray(_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 data.size() * sizeof(float),
                 data.data(),
                 GL_STATIC_DRAW);

    // location 0: position (3 floats) at offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // location 1: color (3 floats) at offset 3 floats into each vertex
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Renderer::beginFrame()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::draw() const
{
    glBindVertexArray(_VAO);
    glDrawArrays(GL_TRIANGLES, 0, _vertexCount);
}

void Renderer::endFrame()
{
    glfwSwapBuffers(_window);
    glfwPollEvents();
}

bool Renderer::shouldClose() const
{
    return glfwWindowShouldClose(_window);
}

GLFWwindow *Renderer::getWindow() const
{
    return _window;
}
