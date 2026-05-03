#include "Renderer.hpp"
#include <iostream>
#include <stdexcept>

Renderer::Renderer(int width, int height, const char *title)
    : _window(nullptr), _VAO(0), _VBO(0), _EBO(0), _indexCount(0)
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
    if (_EBO)
        glDeleteBuffers(1, &_EBO);
    glfwTerminate();
}

void Renderer::uploadMesh(const Mesh &mesh)
{
    _indexCount = static_cast<GLsizei>(mesh.indices.size());

    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_EBO);

    glBindVertexArray(_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.vertices.size() * sizeof(float),
                 mesh.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

void Renderer::beginFrame()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::draw() const
{
    glBindVertexArray(_VAO);
    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, 0);
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
