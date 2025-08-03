#ifndef NADIR_CONTROLLER_H
#define NADIR_CONTROLLER_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "simulation_state.h"

glm::vec3 computeNadirTorque(const SimulationState& state);

#endif
