#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

namespace raytracer {
    class Camera {
    public:
        Camera(float movementSpeed, float mouseSensitivity);
        void update(float dt, GLFWwindow *window);

        glm::mat3 getViewMatrix();

        glm::vec3 getPosition() const { return position; }

        bool hasMoved = false;
        glm::vec2 eulerRotation;
    private:
        glm::vec3 position;
        glm::vec3 up = { 0.f, 1.f, 0.f };
        glm::vec3 forward;
        glm::vec3 right;
        float movementSpeed;
        float mouseSensitivity;
        glm::vec2 lastMousePosition{};
        glm::vec3 lastPosition{};
        glm::vec2 lastEulerRotation{};

        void updateCameraVectors();
    };
}
