#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "graphics/shader.hpp"
#include "Math.hpp"

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Scop", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    float vertices[] = {
        0.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f};

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    Shader shader("shaders/mvp.vert.glsl", "shaders/basic.frag.glsl");

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Model: position the triangle in the world
        Mat4 model(1.0f);
        model = Math::translate(model, Vec3(0.0f, 0.0f, 0.0f));

        // View: place the camera
        Mat4 view = Math::lookAt(
            Vec3(0.0f, 0.0f, 3.0f), // eye: 3 units back
            Vec3(0.0f, 0.0f, 0.0f), // looking at origin
            Vec3(0.0f, 1.0f, 0.0f)  // up
        );

        // Projection: perspective with 45° FOV
        Mat4 projection = Math::perspective(
            toRadians(45.0f), // fov
            800.0f / 600.0f,  // aspect
            0.1f,             // near
            100.0f            // far
        );

        Mat4 mvp = projection * view * model;

        shader.use();
        shader.setMat4("mvp", mvp);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}
