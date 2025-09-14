#include <chrono>

#include "Camera.h"
#include "Window.h"
#include "Shader.h"
#include "glm/gtc/type_ptr.inl"
using raytracer::Window;
using raytracer::Shader;

void renderQuad();

Shader* defaultShader;
Shader* displayShader;
int frameCount = 0;
GLuint fbo;
GLuint accumTexture;
GLuint quadVAO = 0;
GLuint sphereSSBO = 0;
GLuint triangleSSBO = 0;

raytracer::Camera camera = raytracer::Camera(10, 0.08f);

void resetAccumulation() {
    frameCount = 0;
}

static void windowSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    Window::params.width = width;
    Window::params.height = height;
    glGenTextures(1, &accumTexture);
    glBindTexture(GL_TEXTURE_2D, accumTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    resetAccumulation();
}

struct Sphere {
    glm::vec3 pos;
    float radius;
    glm::vec3 color;
    float smoothness;
    glm::vec3 emissiveColor;
    float emissiveStrength;
};

struct Triangle {
    glm::vec4 posA;
    glm::vec4 posB;
    glm::vec4 posC;
    glm::vec4 normalA;
    glm::vec4 normalB;
    glm::vec4 normalC;
    glm::vec3 color;
    float smoothness;
    glm::vec3 emissiveColor;
    float emissiveStrength;
};

std::vector<Sphere> spheres = {
    { glm::vec3(0.0, 0.0, 0.0), 1.0, glm::vec3(1, 1, 1), 1, glm::vec3(0), 0 },
    { glm::vec3(0.0, 2.0, 2.0), 1.0, glm::vec3(0, 0, 1), 0, glm::vec3(1, 1, 1), 4 },
    { glm::vec3(0.0, -21.0, -1.0), 20.0, glm::vec3(0.7, 0.2, 0.6), 0, glm::vec3(0), 0 }
};

std::vector<Triangle> triangles = {
    {
        glm::vec4(0.0, 0.5, -1.0, 0), glm::vec4(3.0, 0.5, -1.0, 0), glm::vec4(0.0, 0.5, -4.0, 0),
        glm::normalize(glm::vec4(0.0, 1.0, 0.0, 0)), glm::normalize(glm::vec4(0.0, 1.0, 0.0, 0)), glm::normalize(glm::vec4(0.0, 1.0, 0.0, 0)),
        glm::vec3(0.2, 0.8, 0.3), 0, glm::vec3(0), 0
    }
};

double deltaTime = 0.0f;
std::chrono::time_point<std::chrono::system_clock> startFrame;

int main() {
    Window window(800, 600);
    glfwSetFramebufferSizeCallback(window.getWindow(), windowSizeCallback);

    defaultShader = new Shader("resources/shaders/default.vert", "resources/shaders/default.frag", "resources/shaders/raytracer.comp");
    displayShader = new Shader("resources/shaders/display.vert", "resources/shaders/display.frag");

    glGenBuffers(1, &sphereSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(Sphere), spheres.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sphereSSBO);

    glGenBuffers(1, &triangleSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, triangles.size() * sizeof(Triangle), triangles.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangleSSBO);

    defaultShader->useCompute();
    defaultShader->setInt("maxBounces", 4, true);
    defaultShader->setInt("samplesPerPixel", 10, true);

    glfwSwapInterval(0);

    while (!glfwWindowShouldClose(window.getWindow())) {
        startFrame = std::chrono::high_resolution_clock::now();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.update(deltaTime, window.getWindow());
        if (camera.hasMoved)
            resetAccumulation();

        // accumulate pass
        defaultShader->useCompute();
        glBindImageTexture(0, accumTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sphereSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangleSSBO);

        defaultShader->setUInt("renderedFrames", frameCount, true);
        defaultShader->setMatrix3x3("cameraRotation", glm::value_ptr(camera.getViewMatrix()), true);
        defaultShader->setVector3("cameraPosition", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z, true);
        defaultShader->setUIVector2("uResolution", Window::params.width, Window::params.height, true);
        defaultShader->setFloat("uFocalLength", static_cast<float>(tan(45.0 / 180.0 * std::numbers::pi)) * 0.5f * static_cast<float>(Window::params.height), true);
        defaultShader->setBool("shouldAccumulate", !camera.hasMoved, true);

        GLuint gx = (Window::params.width  + 7u) / 8u;
        GLuint gy = (Window::params.height + 7u) / 8u;
        glDispatchCompute(gx, gy, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

        // display pass
        displayShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accumTexture);

        renderQuad();

        glfwSwapBuffers(window.getWindow());
        glfwPollEvents();
        frameCount++;
        deltaTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - startFrame).count();
        std::cout << 1/deltaTime << std::endl;
    }
}

GLuint quadVBO = 0;

void renderQuad() {
    if(quadVAO == 0)
    {
        GLuint EBO;

        float vertices[] = {
            1.0f,  1.0f,  1.0, 1.0,  // top right
            1.0f, -1.0f,  1.0, 0.0,  // bottom right
           -1.0f, -1.0f,  0.0, 0.0,  // bottom left
           -1.0f,  1.0f,  0.0, 1.0   // top left
       };
        unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
        };

        glGenVertexArrays(1, &quadVAO);
        glBindVertexArray(quadVAO);

        glGenBuffers(1, &quadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glGenTextures(1, &accumTexture);
        glBindTexture(GL_TEXTURE_2D, accumTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 800, 600, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glBindVertexArray(quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}