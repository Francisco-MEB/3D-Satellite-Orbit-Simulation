#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Shader_s.h"
#include "Camera.h"

inline constexpr int SCR_WIDTH = 800;
inline constexpr int SCR_HEIGHT = 600;

Camera camera{glm::vec3(0.0f, 0.0f, 3.0f)};
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow *window, double xoffset, yoffset);

int main(int argc, char *argv[])
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Container", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to initialize GLFW window" << '\n';
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << '\n';
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader ourShader { "../include/shaders/3.3.shader.vs", "../include/shaders/3.3.shader.fs" };

    const float vertices[] = {
        // front (0-3)
         0.5f,  0.5f, 0.5f, 1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.5f, 1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.5f, 0.0f, 1.0f, // top left

        // back (4-7)
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,

        // left (8-11)
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
        
        // right (12-15)
        0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 0.0f, 1.0f,

        // top (16-19)
         0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
         0.5f, 0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f,  0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,

        // bottom 20-23
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f
    };

    const unsigned int indices[] = {
        // front
        0, 1, 3,
        1, 2, 3,

        // back
        4, 5, 7,
        5, 6, 7,

        // left
        8, 9, 11,
        9, 10, 11,
        
        // right
        12, 13, 15,
        13, 14, 15,
        
        // top
        16, 17, 19,
        17, 18, 19,
        
        // bottom
        20, 21, 23,
        21, 22, 23
    };

    unsigned int VBO{}, EBO{}, VAO{};
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)1);
    glEnableVertexAttribArray(1);

    unsigned int texture1{}, texture2{};
    glGenTextures(1, &texture1);
    glGenTextures(1, &texture2);

    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width{}, height{}, nrChannels{};
    unsigned char *data = stbi_load("../resources/container.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else { std::cerr << "Failed to load texture" << '\n'; }
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("../resources/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else { std::cerr << "Failed to load texture" << '\n'; }
    stbi_image_free(data);

    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);
    
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        ourShader.use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
        unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glm::mat4 view = camera.GetViewMatrix();
        unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        unsigned int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;


    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
