#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "simulation_state.h"

glm::mat4 computeCameraView(const SimulationState& state, Camera& camera);

#endif
