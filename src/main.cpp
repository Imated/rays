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
GLuint accumTextures[2];
GLuint quadVAO = 0;

raytracer::Camera camera = raytracer::Camera(10, 0.08f);

void resetAccumulation() {
    frameCount = 0;
}

void initAccum()
{
    if (quadVAO == 0)
        renderQuad(); // this creates quadVAO, accumTextures[], and fbo

    // init both accum textures to black once
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    for (int i = 0; i < 2; ++i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTextures[i], 0);
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void windowSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    Window::params.width = width;
    Window::params.height = height;
    glBindTexture(GL_TEXTURE_2D, accumTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, accumTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    resetAccumulation();
}

double deltaTime = 0.0f;
std::chrono::time_point<std::chrono::system_clock> startFrame;

int main() {
    Window window(800, 600);
    glfwSetFramebufferSizeCallback(window.getWindow(), windowSizeCallback);

    defaultShader = new Shader("resources/shaders/default.vert", "resources/shaders/default.frag");
    displayShader = new Shader("resources/shaders/display.vert", "resources/shaders/display.frag");
    initAccum();

    while (!glfwWindowShouldClose(window.getWindow())) {
        startFrame = std::chrono::high_resolution_clock::now();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.update(deltaTime, window.getWindow());
        if (camera.hasMoved)
            resetAccumulation();

        int readIdx  = frameCount % 2;
        int writeIdx = (frameCount + 1) % 2;

        // accumulate pass
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTextures[writeIdx], 0);

        defaultShader->use();
        defaultShader->setUInt("renderedFrames", frameCount);
        defaultShader->setInt("maxBounces", 10);
        defaultShader->setInt("samplesPerPixel", 100);
        defaultShader->setMatrix3x3("cameraRotation", glm::value_ptr(camera.getViewMatrix()));
        defaultShader->setVector3("cameraPosition", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
        defaultShader->setBool("shouldAccumulate", !camera.hasMoved);

        defaultShader->setUIVector2("uResolution", Window::params.width, Window::params.height);
        defaultShader->setFloat("uFocalLength", static_cast<float>(tan(45.0 / 180.0 * std::numbers::pi)) * 0.5f * static_cast<float>(Window::params.height));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accumTextures[readIdx]);

        if (frameCount == 0) {
            glClearColor(0,0,0,0);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        renderQuad();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // display pass
        displayShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accumTextures[writeIdx]);

        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window.getWindow());
        glfwPollEvents();
        frameCount++;
        deltaTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - startFrame).count();
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

        glGenTextures(2, accumTextures);
        for (unsigned int t : accumTextures) {
            glBindTexture(GL_TEXTURE_2D, t);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 800, 600, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glGenFramebuffers(1, &fbo);
    }

    glBindVertexArray(quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}