#include "Camera.h"

Camera::Camera(const int width, const int height, const glm::vec3& position):
	startPosition{ position }
{
	Set(width, height, position);
}


void Camera::Set(const int width, const int height, const glm::vec3& position)
{
    this->isPerspective = true;
    this->yaw = YAW;
    this->pitch = PITCH;

    this->FoVy = FOV;
    this->width = width;
    this->height = height;
    this->zNear = zNEAR;
    this->zFar = zFAR;

    this->worldUp = glm::vec3(0, 1, 0);// este y
    this->position = position;

    lastX = width / 2.0f;
    lastY = height / 2.0f;
    bFirstMouseMove = true;

    UpdateCameraVectors();
}

const glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(position, position + forward, up);
}

const glm::mat4 Camera::GetProjectionMatrix() const
{
    glm::mat4 Proj = glm::mat4(1);
    if (isPerspective) {
        float aspectRatio = ((float)(width)) / height;
        Proj = glm::perspective(glm::radians(FoVy), aspectRatio, zNear, zFar);
    }
    else {
        float scaleFactor = 2000.f;
        Proj = glm::ortho<float>(
            -width / scaleFactor, width / scaleFactor,
            -height / scaleFactor, height / scaleFactor, -zFar, zFar);
    }
    return Proj;
}

void Camera::updatePosition(CameraMovement direction, double deltaTime)
{
    float velocity = cameraSpeedFactor * deltaTime;
    switch (direction)
    {
    case CameraMovement::FORWARD:
        this->position += forward * velocity;
        break;
    case CameraMovement::BACKWARD:
        this->position -= forward * velocity;
        break;
    case CameraMovement::RIGHT:
        this->position += right * velocity;
        break;
    case CameraMovement::LEFT:
        this->position -= right * velocity;
        break;
    case CameraMovement::UP:
        this->position += up * velocity;
        break;
    case CameraMovement::DOWN:
        this->position -= up * velocity;
        break;
    default:
        break;
    }
}

void Camera::ResetCamera(int width, int height)
{
    Set(width, height, startPosition);
}

void Camera::MouseControl(float xpos, float ypos)
{
    if (bFirstMouseMove)
    {
        lastX = xpos;
        lastY = ypos;
        bFirstMouseMove = false;
    }

    float xOffset = (xpos - lastX) * MouseSensitivity;
    float yOffset = (lastY - ypos) * MouseSensitivity; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    ProcessMouseMovement(xOffset, yOffset);
}

void Camera::ProcessMouseScroll(float yOffset)
{
    if (FoVy >= 1.0f && FoVy <= 45.0f)
        FoVy -= yOffset;
    if (FoVy <= 1.0f)
        FoVy = 1.0f;
    if (FoVy >= 45.0f)
        FoVy = 45.0f;
}

void Camera::Reshape(int width, int height)
{
	this->width = width;
	this->height = height;
	//lastX = width / 2.0f;
	//lastY = height / 2.0f;
	UpdateCameraVectors();
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
    yaw += xOffset;// stanga deapta
    pitch += yOffset;// sus jos

    // Avem grijã sã nu ne dãm peste cap
    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    // Se modificã vectorii camerei pe baza unghiurilor Euler
    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
    // Calculate the new forward vector
    this->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->forward.y = sin(glm::radians(pitch));
    this->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->forward = glm::normalize(this->forward);
    // Also re-calculate the Right and Up vector
    right = glm::normalize(glm::cross(forward, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    up = glm::normalize(glm::cross(right, forward));
}
