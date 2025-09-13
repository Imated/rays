#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"

namespace raytracer {
    Camera::Camera(float movementSpeed, float mouseSensitivity): position(), up(), forward(), right(), eulerRotation(),movementSpeed(movementSpeed),mouseSensitivity(mouseSensitivity)
    {
        updateCameraVectors();
    }

    void Camera::update(float dt, GLFWwindow* window) {
        lastPosition = position;

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        double dx = (mx - lastMousePosition.x) * mouseSensitivity;
        double dy = (my - lastMousePosition.y) * mouseSensitivity;
        lastMousePosition.x = mx;
        lastMousePosition.y = my;

        eulerRotation.x += static_cast<float>(dx);        // yaw
        eulerRotation.y -= static_cast<float>(dy);        // pitch
        eulerRotation.y = glm::clamp(eulerRotation.y, -89.0f, 89.0f);
        updateCameraVectors();

        glm::vec3 worldUp(0,1,0);
        glm::vec3 horizFwd = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
        glm::vec3 horizRight = glm::normalize(glm::cross(horizFwd, worldUp));

        glm::vec3 move(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += horizFwd;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= horizFwd;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += horizRight;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= horizRight;

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) move += worldUp;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) move -= worldUp;

        if (glm::length(move) > 0.0f)
            position += glm::normalize(move) * movementSpeed * dt;

        hasMoved = (lastPosition != position || lastEulerRotation != eulerRotation);
        lastEulerRotation = eulerRotation;
    }

    glm::mat3 Camera::getViewMatrix() {
        return { right, up, forward };
    }

    void Camera::updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(eulerRotation.x)) * cos(glm::radians(eulerRotation.y));
        front.y = sin(glm::radians(eulerRotation.y));
        front.z = sin(glm::radians(eulerRotation.x)) * cos(glm::radians(eulerRotation.y));
        forward = glm::normalize(front);
        right = glm::normalize(glm::cross(forward, glm::vec3(0.0, 1.0, 0.0)));
        up = glm::normalize(glm::cross(right, forward));
    }
}
