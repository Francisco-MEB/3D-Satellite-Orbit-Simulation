#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "attitude.h"
#include "camera.h"
#include "constants.h"
#include "cube.h"
#include "functions_main.h"
#include "geometric_data.h"
#include "nadir_controller.h"
#include "shader_s.h"
#include "simulation_state.h"
#include "sphere.h"
#include "text_renderer.h"

Camera camera { glm::vec3(0.0f, 0.0f, 50.0f) };

int main(int argc, char *argv[])
{
    SimulationState state;

    declareHints();
    GLFWwindow *window = initWindow(state);
    if (window == nullptr) { return -1; }
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << '\n';
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader skyboxShader { "shaders/skybox.vs", "shaders/skybox.fs" };
    Shader sunShader { "shaders/sun.vs", "shaders/sun.fs" };
    Shader earthShader { "shaders/earth.vs", "shaders/earth.fs" };
    Shader cubesatShader { "shaders/cubesat.vs", "shaders/cubesat.fs" };
    Shader textShader { "shaders/text.vs", "shaders/text.fs" };
   
    // Skybox 
    unsigned int skyboxVAO;
    unsigned int skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxConsts::vertices), SkyboxConsts::vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    std::array<std::string_view, 6> faces
    {
        "skyboxes/right.png",
        "skyboxes/left.png",
        "skyboxes/top.png",
        "skyboxes/bottom.png",
        "skyboxes/front.png",
        "skyboxes/back.png"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // Sun
    Sphere sun(RenderSettings::SPHERE_SECTOR_COUNT, 
               RenderSettings::SPHERE_STACK_COUNT, 
               1.0f);
    unsigned int sunMap = loadTexture("textures/sun.jpg");

    // Earth
    Sphere earth(RenderSettings::SPHERE_SECTOR_COUNT,
                 RenderSettings::SPHERE_STACK_COUNT,
                 1.0f);
    unsigned int earthMap = loadTexture("textures/earthDay.jpg");

    // CubeSat
    CubeSat cubesat {};
    stbi_set_flip_vertically_on_load(true); 
    unsigned int cubesatDiffuseMap = loadTexture("textures/cubesat.png");
    unsigned int cubesatSpecularMap = loadTexture("textures/cubesat_specular.png");
    unsigned int cubesatTopMap = loadTexture("textures/cubesat_top.png");
    unsigned int cubesatBottomMap = loadTexture("textures/cubesat_bottom.png");
    stbi_set_flip_vertically_on_load(false);

    TextRenderer textRenderer(Window::SCR_WIDTH, Window::SCR_HEIGHT);
    textRenderer.Load("fonts/eurostile.TTF", 24);
    glm::mat4 textProjection = glm::ortho(0.0f, (float)Window::SCR_WIDTH, 0.0f, (float)Window::SCR_HEIGHT);
    textShader.use();
    textShader.setMat4("projection", textProjection);

    // Setting textures
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    sunShader.use();
    sunShader.setInt("sunMap", 1);

    earthShader.use();
    earthShader.setInt("earthMap", 2);
    earthShader.setVec3("lightPos", glm::normalize(state.lightPos));
    earthShader.setVec3("viewPos", camera.Position);

    cubesatShader.use();
    cubesatShader.setInt("material.diffuse", 3);
    cubesatShader.setInt("material.specular", 4);
    cubesatShader.setInt("material.topDiffuse", 5);
    cubesatShader.setInt("material.bottomDiffuse", 6);

    glm::vec3 cubesatPosMeters(0.0f, 0.0f, Physics::EARTH_RADIUS + Physics::ORBIT_ALTITUDE);

    glm::vec3 orbitDir = glm::normalize(glm::cross(glm::vec3(0, 1, 0), cubesatPosMeters));
    float orbitVelocity_mps =
       glm::sqrt(Physics::G * Physics::EARTH_MASS /
                (Physics::EARTH_RADIUS + Physics::ORBIT_ALTITUDE));

    state.cubesatVel = orbitVelocity_mps * orbitDir; 

    // To the user: check terminal for detailed sim speed info
    const float SIM_SPEED = getSimSpeed();

    while (!glfwWindowShouldClose(window)) 
    {
        float currentFrame { static_cast<float>(glfwGetTime()) };
        state.deltaTime = currentFrame - state.lastFrame;
        state.lastFrame = currentFrame;
        if (state.deltaTime > MAX_DELTA_TIME)
        {
            state.deltaTime = MAX_DELTA_TIME;
        }

        const float simDeltaTime { state.deltaTime * SIM_SPEED };

        glm::vec3 r_vec = -(state.cubesatPos / SCALE_FACTOR);
        float dist = glm::length(r_vec);
        glm::vec3 grav_dir = glm::normalize(r_vec);
        float force_mag =
            Physics::G * Physics::EARTH_MASS * Physics::CUBESAT_MASS / (dist * dist);
        glm::vec3 accel_old = (force_mag / Physics::CUBESAT_MASS) * grav_dir;

        state.cubesatPos += (state.cubesatVel * SCALE_FACTOR) * simDeltaTime + 0.5f * (accel_old * SCALE_FACTOR) * simDeltaTime * simDeltaTime;

        glm::vec3 r_vec_new = -(state.cubesatPos / SCALE_FACTOR);
        float dist_new = glm::length(r_vec_new);
        glm::vec3 grav_dir_new = glm::normalize(r_vec_new);
        float force_mag_new =
            Physics::G * Physics::EARTH_MASS * Physics::CUBESAT_MASS / (dist_new * dist_new);
        glm::vec3 accel_new = (force_mag_new / Physics::CUBESAT_MASS) * grav_dir_new;

        state.cubesatVel += 0.5f * (accel_old + accel_new) * simDeltaTime;

        glm::vec3 torqueCmd = computeNadirTorque(state);
        state.wheels.applyTorqueCommands(torqueCmd, simDeltaTime);
        state.wheels.update(simDeltaTime);

        glm::vec3 reactionTorque = state.wheels.computeReactionTorque(simDeltaTime);

        updateAttitude(state, reactionTorque);

        processInput(window, state);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view;
        if (state.cameraMode == CameraMode::FREE)
            view = camera.GetViewMatrix();
        else if (state.cameraMode == CameraMode::FOLLOW)
        {
            glm::vec3 velocityDir = glm::normalize(state.cubesatVel);
            glm::vec3 upDir = glm::normalize(state.cubesatPos);
            glm::vec3 sideDir = glm::normalize(glm::cross(upDir, velocityDir));
            glm::vec3 camPos = state.cubesatPos
                             + sideDir * 5.0f
                             + upDir * 2.0f;

            view = glm::lookAt(camPos, state.cubesatPos, upDir);
        }
        else if (state.cameraMode == CameraMode::ONBOARD)
        { 
            glm::vec3 localCamOffset(0.0f, 0.5f, 0.0f);

            glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), state.cubesatPos);
            cubeModel *= glm::mat4_cast(state.cubesatOrientation);

            glm::vec3 camPos = glm::vec3(cubeModel * glm::vec4(localCamOffset, 1.0f));

            glm::vec3 localLookDir(0.0f, 1.0f, 0.0f); 
            glm::vec3 worldLookDir = glm::vec3(cubeModel * glm::vec4(localLookDir, 0.0f));

            glm::vec3 localUp(0.0f, 0.0f, 1.0f);
            glm::vec3 worldUp = glm::vec3(cubeModel * glm::vec4(localUp, 0.0f));

            view = glm::lookAt(camPos, camPos + worldLookDir, worldUp);
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)Window::SCR_WIDTH / (float)Window::SCR_HEIGHT, 0.1f, 1000.0f);
        
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

        sunShader.use();
        sunShader.setMat4("view", view);
        sunShader.setMat4("projection", projection);
        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::translate(sunModel, state.lightPos);
        sunModel = glm::scale(sunModel, glm::vec3(2.0f));
        sunShader.setMat4("model", sunModel);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sunMap);
        sun.draw();

        float earthRotationDegPerSec = (360.0f / secsInDay) * SIM_SPEED;
        earthShader.use();
        earthShader.setMat4("view", view);
        earthShader.setMat4("projection", projection);
        earthShader.setVec3("viewPos", camera.Position);
        glm::mat4 earthModel = glm::mat4(1.0f);
        earthModel = glm::scale(earthModel, glm::vec3(Physics::EARTH_RADIUS_SCALED));
        glm::vec3 tiltAxis = glm::vec3(glm::sin(glm::radians(23.5f)), glm::cos(glm::radians(23.5f)), 0.0f);
        earthModel = glm::rotate(earthModel, glm::radians((float)glfwGetTime() * earthRotationDegPerSec), tiltAxis);
        earthShader.setMat4("model", earthModel);
        earthShader.setVec3("lightPos", state.lightPos);
        earthShader.setVec3("viewPos", camera.Position);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, earthMap);
        earth.draw();

        cubesatShader.use();
        cubesatShader.setMat4("view", view);
        cubesatShader.setMat4("projection", projection);
        cubesatShader.setVec3("light.position", state.lightPos);
        cubesatShader.setVec3("light.ambient", glm::vec3(0.2f));
        cubesatShader.setVec3("light.diffuse", glm::vec3(0.5f));
        cubesatShader.setVec3("light.specular", glm::vec3(1.0f));
        cubesatShader.setVec3("viewPos", camera.Position);
        glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), state.cubesatPos);
        glm::mat4 rotation = glm::mat4_cast(state.cubesatOrientation);
        cubeModel *= rotation;
        cubesatShader.setMat4("model", cubeModel);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, cubesatDiffuseMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, cubesatSpecularMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, cubesatTopMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, cubesatBottomMap);
        cubesatShader.setFloat("material.shininess", 32.0f);
        cubesat.draw();

// ==================== TEXT RENDERING ====================
std::ostringstream torqueStream, angVelStream, altitudeStream, timeStream;

torqueStream << std::fixed << std::setprecision(3);
angVelStream << std::fixed << std::setprecision(3);
timeStream << std::fixed << std::setprecision(2);

// Get torque and angular velocity values
glm::vec3 torque = state.wheels.getTorque();
float angVelX = state.wheels.getWheelAngularVelocity(0);
float angVelY = state.wheels.getWheelAngularVelocity(1);
float angVelZ = state.wheels.getWheelAngularVelocity(2);

// Format strings
torqueStream << "Torque (Nm):     X=" << torque.x << " Y=" << torque.y << " Z=" << torque.z;
angVelStream << "AngVel (rad/s): X=" << angVelX << " Y=" << angVelY << " Z=" << angVelZ;

float altitude = glm::length(state.cubesatPos) / SCALE_FACTOR - Physics::EARTH_RADIUS;
altitudeStream << "Altitude (m): " << altitude;

float realTime = static_cast<float>(glfwGetTime());
timeStream << "Time Elapsed: " << realTime << "s";

// Helper for text width
auto calcTextWidth = [&](const std::string& text, float scale) {
    float width = 0.0f;
    for (char c : text) {
        Character ch = textRenderer.Characters[c];
        width += (ch.Advance >> 6) * scale;
    }
    return width;
};

float scale = 0.8f;

// ---- Bottom-left (Torque) ----
float bottomYOffset = 30.0f; // distance from bottom edge
float lineSpacing = 20.0f;   // vertical gap between lines
float leftX = 10.0f;         // fixed margin from left

textRenderer.RenderText(textShader, torqueStream.str(),
                        leftX,
                        bottomYOffset + lineSpacing,
                        scale, glm::vec3(1.0f));

textRenderer.RenderText(textShader, angVelStream.str(),
                        leftX,
                        bottomYOffset,
                        scale, glm::vec3(1.0f));

// ---- Top-left (Altitude) ----
textRenderer.RenderText(textShader, altitudeStream.str(),
                        10.0f, Window::SCR_HEIGHT - 30.0f,
                        scale, glm::vec3(1.0f));

// ---- Top-right (Elapsed Time) ----
float timeWidth = calcTextWidth(timeStream.str(), scale);
textRenderer.RenderText(textShader, timeStream.str(),
                        Window::SCR_WIDTH - timeWidth - 10.0f, Window::SCR_HEIGHT - 30.0f,
                        scale, glm::vec3(1.0f));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}

