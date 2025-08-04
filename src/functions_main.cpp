#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <iostream>
#include <string>
#include <vector>

#include "attitude.h"
#include "camera.h"
#include "functions_main.h"
#include "nadir_controller.h"
#include "simulation_state.h"

float lastX { Window::SCR_WIDTH / 2.0 };
float lastY { Window::SCR_HEIGHT / 2.0 };
bool firstMouse { true };

bool cKeyPressedLastFrame { false };

void initNadirPointing(SimulationState& state)
{
    glm::vec3 r = state.cubesatPos;
    glm::vec3 v = state.cubesatVel;

    glm::vec3 z_dir = glm::normalize(-r);
    glm::vec3 h = glm::normalize(glm::cross(r, v));
    glm::vec3 x_dir = glm::normalize(glm::cross(h, z_dir));
    glm::vec3 y_dir = glm::cross(z_dir, x_dir);           

    glm::mat3 R_desired(x_dir, y_dir, z_dir);
    state.cubesatOrientation = glm::normalize(glm::quat_cast(R_desired));

    float mu = Physics::G * Physics::EARTH_MASS;
    float r_unscaled = glm::length(r) / SCALE_FACTOR;
    float orbitalRate = std::sqrt(mu / (r_unscaled * r_unscaled * r_unscaled));

    state.cubesatAngularVel = R_desired * glm::vec3(0.0f, orbitalRate, 0.0f);
}

void renderSkybox(Shader& skyboxShader, const glm::mat4& view, const glm::mat4& projection, 
                  unsigned int skyboxVAO, unsigned int cubemapTexture)
{
    glDepthFunc(GL_LEQUAL);

    glm::mat4 viewSkybox = glm::mat4(glm::mat3(view));
    skyboxShader.use();
    skyboxShader.setMat4("view", viewSkybox);
    skyboxShader.setMat4("projection", projection);

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
    glDepthFunc(GL_LESS); 
}

void renderSun(Shader& sunShader, const glm::mat4& view, const glm::mat4& projection,
               const glm::vec3& lightPos)
{
    sunShader.use();
    sunShader.setMat4("view", view);
    sunShader.setMat4("projection", projection);
    glm::mat4 sunModel = glm::mat4(1.0f);
    sunModel = glm::translate(sunModel, lightPos);
    sunModel = glm::scale(sunModel, glm::vec3(2.0f));
    sunShader.setMat4("model", sunModel);
}

void renderEarth(Shader& earthShader, const glm::mat4& view, const glm::mat4& projection,
                 const glm::vec3& lightPos, const glm::vec3& cameraPos, float simSpeed)
{
    float earthRotationDegPerSec = (360.0f / SECS_IN_DAY) * simSpeed;

    earthShader.use();
    earthShader.setMat4("view", view);
    earthShader.setMat4("projection", projection);
    earthShader.setVec3("viewPos", cameraPos);

    glm::mat4 earthModel = glm::mat4(1.0f);
    earthModel = glm::scale(earthModel, glm::vec3(Physics::EARTH_RADIUS_SCALED));

    glm::vec3 tiltAxis = glm::vec3(glm::sin(glm::radians(23.5f)),
                                   glm::cos(glm::radians(23.5f)), 0.0f);

    earthModel = glm::rotate(
        earthModel,
        glm::radians((float)glfwGetTime() * earthRotationDegPerSec),
        tiltAxis
    );

    earthShader.setMat4("model", earthModel);
    earthShader.setVec3("lightPos", lightPos); 
}

void renderCubesat(Shader& cubesatShader, const glm::mat4& view, const glm::mat4& projection,
                   const glm::vec3& lightPos, const glm::vec3& cameraPos, const glm::vec3& cubesatPos,
                   const glm::quat& cubesatOrientation)
{
    cubesatShader.use();
    cubesatShader.setMat4("view", view);
    cubesatShader.setMat4("projection", projection);

    cubesatShader.setVec3("light.position", lightPos);
    cubesatShader.setVec3("light.ambient", glm::vec3(0.2f));
    cubesatShader.setVec3("light.diffuse", glm::vec3(0.5f));
    cubesatShader.setVec3("light.specular", glm::vec3(1.0f));
    cubesatShader.setVec3("viewPos", cameraPos);

    glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), cubesatPos);
    glm::mat4 rotation = glm::mat4_cast(cubesatOrientation);
    cubeModel *= rotation;

    cubesatShader.setMat4("model", cubeModel);
    cubesatShader.setFloat("material.shininess", 32.0f);
}

// Verlet integration is used to compute position and velocity
void propagateOrbit(SimulationState& state, float dt)
{
    // Gravity points downward to Earth
    // Unscale pos value for physics calculations
    glm::vec3 r_vec = -(state.cubesatPos / SCALE_FACTOR);
    float dist = glm::length(r_vec);
    glm::vec3 grav_dir = glm::normalize(r_vec);
    float force_mag =
        Physics::G * Physics::EARTH_MASS * Physics::CUBESAT_MASS
        / std::pow(dist, 2);
    glm::vec3 accel_i = (force_mag / Physics::CUBESAT_MASS) * grav_dir;

    // Update position
    state.cubesatPos += (state.cubesatVel * SCALE_FACTOR) * dt + 0.5f * (accel_i * SCALE_FACTOR) * static_cast<float>(std::pow(dt, 2));

    // Compute new acceleration
    glm::vec3 r_vec_f = -(state.cubesatPos / SCALE_FACTOR);
    float dist_f = glm::length(r_vec_f);
    glm::vec3 grav_dir_f = glm::normalize(r_vec_f);
    float force_mag_f = 
        Physics::G * Physics::EARTH_MASS * Physics::CUBESAT_MASS
        / std::pow(dist_f, 2);
    glm::vec3 accel_f = (force_mag_f / Physics::CUBESAT_MASS) * grav_dir_f;

    // Update velocity
    state.cubesatVel += 0.5f * (accel_i + accel_f) * dt;
}

void updateAttitudeControl(SimulationState& state, float dt)
{
    glm::vec3 torqueCmd = computeNadirTorque(state);
    state.wheels.applyTorqueCommands(torqueCmd, dt);
    state.wheels.update(dt);

    glm::vec3 reactionTorque = state.wheels.computeReactionTorque(dt);
    updateAttitude(state, reactionTorque);
}

// Use non-scaled values in physics calculations
glm::vec3 calculateCubesatVel()
{
    glm::vec3 cubesatPosMeters(0.0f, 0.0f, Physics::EARTH_RADIUS + Physics::ORBIT_ALTITUDE);

    glm::vec3 orbitDir = glm::normalize(glm::cross(glm::vec3(0, 1, 0), cubesatPosMeters));
    
    float orbitSpeed_mps = 
        glm::sqrt(Physics::G * Physics::EARTH_MASS / (Physics::EARTH_RADIUS + Physics::ORBIT_ALTITUDE));

    return orbitSpeed_mps * orbitDir;
}

void updateDeltaTime(SimulationState& state)
{
    float currentFrame { static_cast<float>(glfwGetTime()) };
    state.deltaTime = currentFrame - state.lastFrame;
    state.lastFrame = currentFrame;
    if (state.deltaTime > MAX_DELTA_TIME)
    {
        state.deltaTime = MAX_DELTA_TIME;
    }
} 

void declareHints()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

GLFWwindow *initWindow(SimulationState& state)
{
    GLFWwindow *window = glfwCreateWindow(Window::SCR_WIDTH, Window::SCR_HEIGHT, "CubeSat Simulation", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to initialize GLFW window" << '\n';
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetWindowUserPointer(window, &state);

    return window;
}

void processInput(GLFWwindow *window, [[maybe_unused]] SimulationState& state)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (state.cameraMode == CameraMode::FREE)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, state.deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, state.deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, state.deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, state.deltaTime);
    }

    bool cKeyPressed = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    if (cKeyPressed && !cKeyPressedLastFrame)
    {
        if (state.cameraMode == CameraMode::FREE)
            state.cameraMode = CameraMode::FOLLOW;
        else if (state.cameraMode == CameraMode::FOLLOW)
            state.cameraMode = CameraMode::ONBOARD;
        else
            state.cameraMode = CameraMode::FREE;

        std::cout << "Camera mode changed to " << static_cast<int>(state.cameraMode) << '\n';
    }
    cKeyPressedLastFrame = cKeyPressed;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    auto *state { static_cast<SimulationState*>(glfwGetWindowUserPointer(window)) };
    if (!state) { return; }

    float xpos { static_cast<float>(xposIn) };
    float ypos { static_cast<float>(yposIn) };

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset { xpos - lastX };
    float yoffset { lastY - ypos };

    lastX = xpos;
    lastY = ypos;
    
    if (state->cameraMode == CameraMode::FREE)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1) { format = GL_RED; }
        else if (nrChannels == 3) { format = GL_RGB; }
        else if (nrChannels == 4) { format = GL_RGBA; }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else { std::cerr << "Texture failed to load at path: " << path << '\n'; }

    stbi_image_free(data);

    return textureID;
}

unsigned int loadCubemap(const std::array<std::string_view, 6>& faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (int i = 0; i < static_cast<int>(faces.size()); ++i)
    {
        unsigned char *data = stbi_load(faces[i].data(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format;
            if (nrChannels == 1) { format = GL_RED; }
            else if (nrChannels == 3) { format = GL_RGB; }
            else if (nrChannels == 4) { format = GL_RGBA; }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        } 
        else { std::cerr << "Cubemap texture failed to load at path: " << faces[i] << '\n'; }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void ignoreLine()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

float getSimSpeed()
{
    float simSpeed {};
    do
    {
        if (std::cin.fail()) 
        { 
            std::cin.clear();
            ignoreLine(); 
        }

        std::cout << "Choose SIM_SPEED:\n";
        std::cout << "---------------------------------------------\n";
        std::cout << "SIM_SPEED | 1 Earth Rotation | 1 Orbit Time\n";
        std::cout << "---------------------------------------------\n";
        std::cout << "1         | 24 hours         | 92 minutes\n";
        std::cout << "60        | 24 minutes       | 1.53 minutes\n";
        std::cout << "600       | 2.4 minutes      | 9.2 seconds\n";
        std::cout << "3600      | 24 seconds       | 1.53 minutes\n";
        std::cout << "8640      | 10 seconds       | 38 seconds\n";
        std::cout << "86400     | 1 second         | 5.5 seconds\n";
        std::cout << "---------------------------------------------\n"; 

        std::cout << "Enter desired SIM_SPEED: ";
        std::cin >> simSpeed; 
    } while (std::cin.fail());

    return simSpeed;
}
