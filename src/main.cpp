#include "Renderer.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <iostream>
#include "ObjParser.hpp"
#include "Math.hpp"
#include "ModelTransform.hpp"
#include "Texture.hpp"
#include <filesystem>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: ./scop <model.obj> [texture.bmp]\n";
        return 1;
    }

    try
    {
        Mesh mesh = ObjParser::parse(argv[1]);

        const char *texturePath = (argc >= 3) ? argv[2] : "textures/default.bmp";
        if (!std::filesystem::is_regular_file(texturePath))
        {
            throw std::runtime_error("Could not open texture: " + std::string(texturePath));
        }

        Renderer renderer(800, 600, "Scop");
        renderer.uploadMesh(mesh);

        Shader shader("shaders/mvp.vert.glsl", "shaders/basic.frag.glsl");

        Texture texture(texturePath);

        shader.use();
        shader.setInt("tex", 0);

        Camera camera(Vec3(0.0f, 0.0f, 7.0f));
        ModelTransform model;

        Vec3 meshCenter = mesh.center();
        float meshExtent = mesh.extent();
        float scaleFactor = (meshExtent > 0.0f) ? 1.0f / meshExtent : 1.0f;

        Mat4 projection = Math::perspective(toRadians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        bool showTexture = false;
        float texMix = 0.0f;
        int tPrev = GLFW_RELEASE;
        const float fadeDuration = 0.3f;

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

            int tNow = glfwGetKey(renderer.getWindow(), GLFW_KEY_T);
            if (tNow == GLFW_PRESS && tPrev == GLFW_RELEASE)
                showTexture = !showTexture;
            tPrev = tNow;

            float target = showTexture ? 1.0f : 0.0f;
            float step = deltaTime / fadeDuration;
            if (texMix < target)
                texMix = (texMix + step < target) ? texMix + step : target;
            else if (texMix > target)
                texMix = (texMix - step > target) ? texMix - step : target;

            camera.processInput(renderer.getWindow(), deltaTime);
            model.processInput(renderer.getWindow(), deltaTime);

            Mat4 mvp = projection * camera.getViewMatrix() * model.getModelMatrix(meshCenter, scaleFactor);

            renderer.beginFrame();
            shader.use();
            shader.setMat4("mvp", mvp);
            shader.setFloat("useTexture", texMix);
            texture.bind(0);
            renderer.draw();
            renderer.endFrame();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
