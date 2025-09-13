#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"

namespace raytracer {
    Camera::Camera(float movementSpeed, float mouseSensitivity): position(), up(), forward(), right(), eulerRotation(),
                                                                 movementSpeed(movementSpeed),
                                                                 mouseSensitivity(mouseSensitivity),
                                                                 lastMousePosition() { updateCameraVectors();}

    void Camera::update(float dt, GLFWwindow* window) {
        lastPosition = position;
        if (glfwGetKey(window, GLFW_KEY_SPACE))
            position.y += movementSpeed * dt;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
            position.y -= movementSpeed * dt;
        if (glfwGetKey(window, GLFW_KEY_D))
            position.x += movementSpeed * dt;
        if (glfwGetKey(window, GLFW_KEY_A))
            position.x -= movementSpeed * dt;
        if (glfwGetKey(window, GLFW_KEY_W))
            position.z += movementSpeed * dt;
        if (glfwGetKey(window, GLFW_KEY_S))
            position.z -= movementSpeed * dt;
        double mouseX, mouseY = 0;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        double mouseDeltaX = (mouseX - lastMousePosition.x) * mouseSensitivity;
        double mouseDeltaY = (mouseY - lastMousePosition.y) * mouseSensitivity;
        lastMousePosition = glm::vec2(mouseX, mouseY);

        lastEulerRotation = eulerRotation;
        eulerRotation.x += mouseDeltaX;
        eulerRotation.y += mouseDeltaY;
        if (eulerRotation.y > 89.0f)
            eulerRotation.y = 89.0f;
        else if (eulerRotation.y < -89.0f)
            eulerRotation.y = -89.0f;
        if (lastEulerRotation == eulerRotation && lastPosition == position)
            hasMoved = false;
        else
            hasMoved = true;
        updateCameraVectors();
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(position, position + forward, up);
    }

    void Camera::updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(eulerRotation.x)) * cos(glm::radians(eulerRotation.y));
        front.y = sin(glm::radians(eulerRotation.y));
        front.z = sin(glm::radians(eulerRotation.x)) * cos(glm::radians(eulerRotation.y));
        forward = glm::normalize(front);
        right = glm::normalize(glm::cross(forward, up));
        up = glm::normalize(glm::cross(right, forward));
    }
}
