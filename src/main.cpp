#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include "shader.hpp"
#include "ObjParser.hpp"
#include "Math.hpp"

// Camera state
static Vec3 cameraPos(0.0f, 0.0f, 7.0f);
static float yaw = -90.0f;
static float pitch = 0.0f;

// Model rotation state
static float modelYaw = 0.0f;
static float modelPitch = 0.0f;
static float modelRoll = 0.0f;

static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

static Vec3 getCameraFront()
{
    float yawRad = toRadians(yaw);
    float pitchRad = toRadians(pitch);
    return Vec3(
        cosf(pitchRad) * cosf(yawRad),
        sinf(pitchRad),
        cosf(pitchRad) * sinf(yawRad));
}

static void processInput(GLFWwindow *window)
{
    float speed = 2.5f * deltaTime;
    float rotSpeed = 90.0f * deltaTime;

    Vec3 front = getCameraFront();
    Vec3 up(0.0f, 1.0f, 0.0f);
    Vec3 right = front.cross(up).normalized();

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Translation
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos = cameraPos - front * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos = cameraPos + front * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos = cameraPos + right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos = cameraPos - right * speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPos = cameraPos - up * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos = cameraPos + up * speed;

    // Model rotation (arrow keys)
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        modelYaw -= rotSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        modelYaw += rotSpeed;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        modelPitch -= rotSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        modelPitch += rotSpeed;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        modelRoll -= rotSpeed;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        modelRoll += rotSpeed;
}

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

    // Window setup
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

    // Upload mesh to GPU
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

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Model: center at origin, scale, then rotate via arrow keys
        Mat4 model(1.0f);
        model = Math::rotate(model, toRadians(modelYaw), Vec3(0.0f, 1.0f, 0.0f));
        model = Math::rotate(model, toRadians(modelPitch), Vec3(1.0f, 0.0f, 0.0f));
        model = Math::rotate(model, toRadians(modelRoll), Vec3(0.0f, 0.0f, 1.0f));
        model = Math::scale(model, Vec3(scaleFactor, scaleFactor, scaleFactor));
        model = Math::translate(model, Vec3(-meshCenter.x, -meshCenter.y, -meshCenter.z));

        // View: camera with free movement
        Vec3 front = getCameraFront();
        Mat4 view = Math::lookAt(cameraPos, cameraPos + front, Vec3(0.0f, 1.0f, 0.0f));

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
