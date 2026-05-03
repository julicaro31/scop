#include "Renderer.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <iostream>
#include "ObjParser.hpp"
#include "Math.hpp"
#include "ModelTransform.hpp"

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

    Renderer renderer(800, 600, "Scop");
    renderer.uploadMesh(mesh);

    Shader shader("shaders/mvp.vert.glsl", "shaders/basic.frag.glsl");
    Camera camera(Vec3(0.0f, 0.0f, 7.0f));
    ModelTransform model;

    Vec3 meshCenter = mesh.center();
    float meshExtent = mesh.extent();
    float scaleFactor = (meshExtent > 0.0f) ? 1.0f / meshExtent : 1.0f;

    Mat4 projection = Math::perspective(toRadians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    float lastFrame = 0.0f;
    while (!renderer.shouldClose())
    {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(renderer.getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(renderer.getWindow(), true);
        }

        camera.processInput(renderer.getWindow(), deltaTime);
        model.processInput(renderer.getWindow(), deltaTime);

        Mat4 mvp = projection * camera.getViewMatrix() * model.getModelMatrix(meshCenter, scaleFactor);

        renderer.beginFrame();
        shader.use();
        shader.setMat4("mvp", mvp);
        renderer.draw();
        renderer.endFrame();
    }

    return 0;
}
