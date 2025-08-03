#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

int main()
{
    glfwInit();

    // Tell GLFW that 3.3 is the OpenGL version we want to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    // Tell GLFW that we want to use the core profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(800, 500, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << '\n';
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD before calling any OpenGL function; GLAD manages function pointers for OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << '\n';
        return -1;
    }

    // Tell OpenGL the sizes of the rendering window
    glViewport(0, 0, 800, 600);
    // Tell GLFW to call this function on every window resize 
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Keep running until explicitly told to stop
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // rendering commands here
        // ...
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
