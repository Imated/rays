#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

namespace raytracer {
    class Camera {
    public:
        Camera(float movementSpeed, float mouseSensitivity);
        void update(float dt, GLFWwindow *window);
        glm::mat4 getViewMatrix();

        glm::vec3 getPosition() const { return position; }

        bool hasMoved = false;
    private:
        glm::vec3 position;
        glm::vec3 up;
        glm::vec3 forward;
        glm::vec3 right;
        glm::vec2 eulerRotation;
        float movementSpeed;
        float mouseSensitivity;
        glm::vec2 lastMousePosition;
        glm::vec3 lastPosition{};
        glm::vec2 lastEulerRotation{};

        void updateCameraVectors();
    };
}
