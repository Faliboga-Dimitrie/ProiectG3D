#pragma once

#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h>

#include <GL/glew.h>

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>
#include "CameraMovement.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

class Camera
{
    const float zNEAR = 0.1f;
    const float zFAR = 500.f;
    const float YAW = -90.0f;
    const float PITCH = 0.0f;
    const float FOV = 45.0f;
    const float MouseSensitivity = 0.5f;
    glm::vec3 startPosition;

protected:
    const float cameraSpeedFactor = 200.5f;
    const float mouseSensitivity = 0.1f;

    // Perspective properties
    float zNear;
    float zFar;
    float FoVy;
    int width;
    int height;
    bool isPerspective;

    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 worldUp;

    // Euler Angles
    float yaw;
    float pitch;

    bool bFirstMouseMove = true;
    float lastX = 0.f, lastY = 0.f;

public:

    Camera(const int width, const int height, const glm::vec3& position);
    void Set(const int width, const int height, const glm::vec3& position);
    const glm::mat4 GetViewMatrix() const;
    const glm::mat4 GetProjectionMatrix() const;
    void updatePosition(CameraMovement direction, double deltaTime);
    void ResetCamera(int width, int height);
    void MouseControl(float xpos, float ypos);
    void ProcessMouseScroll(float yOffset);
    void Reshape(int width, int height);

private:

    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void UpdateCameraVectors();
};

