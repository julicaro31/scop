#pragma once

#include <GLFW/glfw3.h>

struct ModelTransform
{
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    void processInput(GLFWwindow *window, float deltaTime)
    {
        float rotSpeed = 90.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            yaw -= rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            yaw += rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            pitch -= rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            pitch += rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            roll -= rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            roll += rotSpeed;
    }

    Mat4 getModelMatrix(const Vec3 &center, float scale) const
    {
        Mat4 m(1.0f);
        m = Math::rotate(m, toRadians(yaw), Vec3(0.0f, 1.0f, 0.0f));
        m = Math::rotate(m, toRadians(pitch), Vec3(1.0f, 0.0f, 0.0f));
        m = Math::rotate(m, toRadians(roll), Vec3(0.0f, 0.0f, 1.0f));
        m = Math::scale(m, Vec3(scale, scale, scale));
        m = Math::translate(m, Vec3(-center.x, -center.y, -center.z));
        return m;
    }
};