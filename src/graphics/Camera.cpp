#include "Camera.hpp"
#include <GLFW/glfw3.h>
#include <cmath>

Camera::Camera(const Vec3 &position, float yaw, float pitch)
    : _position(position), _yaw(yaw), _pitch(pitch)
{
}

Vec3 Camera::getFront() const
{
    float yawRad = toRadians(_yaw);
    float pitchRad = toRadians(_pitch);
    return Vec3(
        cosf(pitchRad) * cosf(yawRad),
        sinf(pitchRad),
        cosf(pitchRad) * sinf(yawRad));
}

void Camera::processInput(GLFWwindow *window, float deltaTime)
{
    float speed = 2.5f * deltaTime;

    Vec3 front = getFront();
    Vec3 up(0.0f, 1.0f, 0.0f);
    Vec3 right = front.cross(up).normalized();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        _position = _position - front * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        _position = _position + front * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        _position = _position + right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        _position = _position - right * speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        _position = _position - up * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        _position = _position + up * speed;
}

Mat4 Camera::getViewMatrix() const
{
    Vec3 front = getFront();
    return Math::lookAt(_position, _position + front, Vec3(0.0f, 1.0f, 0.0f));
}
