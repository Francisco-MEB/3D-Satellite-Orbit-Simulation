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
#include "camera_controller.h"
#include "constants.h"
#include "cube.h"
#include "functions_main.h"
#include "geometric_data.h"
#include "nadir_controller.h"
#include "shader_s.h"
#include "simulation_state.h"
#include "sphere.h"
#include "telemetry_display.h"
#include "text_renderer.h"

Camera camera { glm::vec3(0.0f, 0.0f, 50.0f) };

int main(int argc, char *argv[])
{
    SimulationState state; 
    state.cubesatVel = calculateCubesatVel();
    initNadirPointing(state);

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

    // Load font
    TextRenderer textRenderer(Window::SCR_WIDTH, Window::SCR_HEIGHT);
    textRenderer.Load("fonts/ShareTechMono.ttf", FONT_SIZE);
    glm::mat4 textProjection = glm::ortho(0.0f, (float)Window::SCR_WIDTH, 0.0f, (float)Window::SCR_HEIGHT);
    textShader.use();
    textShader.setMat4("projection", textProjection);

    // Telemetry display
    TelemetryDisplay telemetry(textRenderer, textShader);
    telemetry.collateEntries();

    // Setting textures (not abstracting away to keep texture unit indices visible)
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

    // To the user: check terminal for detailed sim speed info
    const float SIM_SPEED = getSimSpeed();

    while (!glfwWindowShouldClose(window)) 
    {
         
        updateDeltaTime(state);

        const float simDeltaTime = state.deltaTime * SIM_SPEED;
        const int subSteps = 15;  
        float subDt = simDeltaTime / static_cast<float>(subSteps);
        for (int i = 0; i < subSteps; ++i)
        {
            propagateOrbit(state, subDt);          
            updateAttitudeControl(state, subDt);  
        }
        state.simElapsedTime += simDeltaTime;
  
        processInput(window, state);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = computeCameraView(state, camera);
        glm::mat4 projection = glm::perspective(
                glm::radians(camera.Zoom), (float)Window::SCR_WIDTH / (float)Window::SCR_HEIGHT, 0.1f, 1000.0f);
   
        // Skybox 
        renderSkybox(skyboxShader, view, projection, skyboxVAO, cubemapTexture);

        // Sun 
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sunMap);
        renderSun(sunShader, view, projection, state.lightPos);
        sun.draw();

        // Earth
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, earthMap);
        renderEarth(earthShader, view, projection, 
                    state.lightPos, camera.Position, SIM_SPEED);
        earth.draw();

        // Cubesat
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, cubesatDiffuseMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, cubesatSpecularMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, cubesatTopMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, cubesatBottomMap);
        renderCubesat(cubesatShader, view, projection, 
                      state.lightPos, camera.Position, state.cubesatPos, 
                      state.cubesatOrientation);
        cubesat.draw();

        // Telemetry
        telemetry.updateAndRender(
            state.wheels.getTorque(),
            state.wheels.getWheelAngularVelocity(0),
            state.wheels.getWheelAngularVelocity(1),
            state.wheels.getWheelAngularVelocity(2),
            glm::length(state.cubesatPos) / SCALE_FACTOR -  Physics::EARTH_RADIUS,
            state.simElapsedTime,
            Window::SCR_WIDTH,
            Window::SCR_HEIGHT
        ); 

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}

