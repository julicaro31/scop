#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Math.hpp"

struct GLFWwindow;

class Camera
{
public:
    Camera(const Vec3 &position, float yaw = -90.0f, float pitch = 0.0f);
    void processInput(GLFWwindow *window, float deltaTime);
    Mat4 getViewMatrix() const;

private:
    Vec3 _position;
    float _yaw;
    float _pitch;
    Vec3 getFront() const;
};

#endif
