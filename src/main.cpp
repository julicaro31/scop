#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader.hpp"
#include "ObjParser.hpp"
#include "Math.hpp"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: ./scop <model.obj>\n";
        return 1;
    }

    Mesh mesh;
    try
    {
        mesh = ObjParser::parse(argv[1]);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }

    //Window setup
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

    glEnable(GL_DEPTH_TEST);

    //Upload mesh to GPU
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.vertices.size() * sizeof(float),
                 mesh.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(),
                 GL_STATIC_DRAW);

    // Position attribute (location = 0): 3 floats per vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    Shader shader("shaders/mvp.vert.glsl", "shaders/basic.frag.glsl");

    Vec3 meshCenter = mesh.center();
    float meshExtent = mesh.extent();
    float scaleFactor = (meshExtent > 0.0f) ? 1.0f / meshExtent : 1.0f;

    //Render loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = glfwGetTime();

        // Model: center the mesh at origin, scale it, then rotate
        Mat4 model(1.0f);
        model = Math::rotate(model, time, Vec3(0.0f, 1.0f, 0.0f));
        model = Math::scale(model, Vec3(scaleFactor, scaleFactor, scaleFactor));
        model = Math::translate(model, Vec3(-meshCenter.x, -meshCenter.y, -meshCenter.z));

        // View: place the camera
        Mat4 view = Math::lookAt(
            Vec3(0.0f, 0.0f, 3.0f),
            Vec3(0.0f, 0.0f, 0.0f),
            Vec3(0.0f, 1.0f, 0.0f));

        // Projection: perspective with 45° FOV
        Mat4 projection = Math::perspective(
            toRadians(45.0f),
            800.0f / 600.0f,
            0.1f,
            100.0f);

        Mat4 mvp = projection * view * model;

        shader.use();
        shader.setMat4("mvp", mvp);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(mesh.indices.size()),
                       GL_UNSIGNED_INT,
                       0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}
