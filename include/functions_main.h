#ifndef FUNCTIONS_MAIN_H
#define FUNCTIONS_MAIN_H

#include <GLFW/glfw3.h>

#include "camera.h"
#include "shader_s.h"
#include "simulation_state.h"

extern Camera camera;

namespace Window
{
    inline constexpr unsigned int SCR_WIDTH { 800 };
    inline constexpr unsigned int SCR_HEIGHT { 600 };
}

void renderSkybox(Shader& skyboxShader, const glm::mat4& view, const glm::mat4& projection,
                  unsigned int skyboxVAO, unsigned int cubemapTexture);
void renderSun(Shader& sunShader, const glm::mat4& view, const glm::mat4& projection,
               const glm::vec3& lightPos); 
void renderEarth(Shader& earthShader, const glm::mat4& view, const glm::mat4& projection,
                 const glm::vec3& lightPos, const glm::vec3& cameraPos, float simSpeed);
void renderCubesat(Shader& cubesatShader, const glm::mat4& view, const glm::mat4& projection,
                   const glm::vec3& lightPos, const glm::vec3& cameraPos, const glm::vec3& cubesatPos,
                   const glm::quat& cubesatOrientation); 
glm::vec3 calculateCubesatVel();
void updateDeltaTime(SimulationState& state);
void updateAttitudeControl(SimulationState& state, float dt);
void propagateOrbit(SimulationState& state, float dt);
void declareHints();
GLFWwindow *initWindow(SimulationState& state);
void processInput(GLFWwindow *window, [[maybe_unused]] SimulationState& state);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
unsigned int loadTexture(char const * path);
unsigned int loadCubemap(const std::array<std::string_view, 6>& faces);
void ignoreLine();
float getSimSpeed();

#endif
