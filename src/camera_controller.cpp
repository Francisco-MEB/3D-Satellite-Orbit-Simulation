#include "camera_controller.h"

glm::mat4 computeCameraView(const SimulationState& state, Camera& camera)
{
    if (state.cameraMode == CameraMode::FREE)
    {
        return camera.GetViewMatrix();
    }
    else if (state.cameraMode == CameraMode::FOLLOW)
    {
        glm::vec3 velocityDir = glm::normalize(state.cubesatVel);
        glm::vec3 upDir = glm::normalize(state.cubesatPos);
        glm::vec3 sideDir = glm::normalize(glm::cross(upDir, velocityDir));

        glm::vec3 camPos = state.cubesatPos + sideDir * 5.0f + upDir * 2.0f;

        return glm::lookAt(camPos, state.cubesatPos, upDir);
    }
    else if (state.cameraMode == CameraMode::ONBOARD)
    {
        glm::vec3 localCamOffset(0.0f, 0.0f, 0.5f);

        glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), state.cubesatPos);
        cubeModel *= glm::mat4_cast(state.cubesatOrientation);

        glm::vec3 camPos = glm::vec3(cubeModel * glm::vec4(localCamOffset, 1.0f));

        glm::vec3 localLookDir(0.0f, 0.0f, 1.0f);
        glm::vec3 worldLookDir = glm::vec3(cubeModel * glm::vec4(localLookDir, 0.0f));

        glm::vec3 localUp(0.0f, 1.0f, 0.0f);
        glm::vec3 worldUp = glm::vec3(cubeModel * glm::vec4(localUp, 0.0f));

        return glm::lookAt(camPos, camPos + worldLookDir, worldUp);
    }

    return camera.GetViewMatrix();
}
