#ifndef ATTITUDE_H
#define ATTITUDE_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "simulation_state.h"

inline const glm::mat3 CUBESAT_INERTIA = glm::mat3(
    0.0010f, 0.0f,    0.0f,
    0.0f,    0.0012f, 0.0f,
    0.0f,    0.0f,    0.0011f
);

inline const glm::mat3 INV_INERTIA = glm::inverse(CUBESAT_INERTIA);

void updateAttitude(SimulationState& state, const glm::vec3& appliedTorque, float simDeltaTime);

#endif

