#define GLM_ENABLE_EXPERIMENTAL
#include "attitude.h"
#include <glm/gtx/quaternion.hpp>

void updateAttitude(SimulationState& state, const glm::vec3& appliedTorque, float simDeltaTime)
{
    glm::vec3 omega = state.cubesatAngularVel;

    glm::vec3 angularMomentum = CUBESAT_INERTIA * omega;

    glm::vec3 omegaCrossH = glm::cross(omega, angularMomentum);

    glm::vec3 angularAcc = INV_INERTIA * (appliedTorque - omegaCrossH);

    // Integrate angular velocity
    state.cubesatAngularVel += angularAcc * simDeltaTime;

    // Quaternion derivative
    glm::quat omegaQuat(0.0f, state.cubesatAngularVel.x,
                                 state.cubesatAngularVel.y,
                                 state.cubesatAngularVel.z);
    glm::quat qDot = 0.5f * omegaQuat * state.cubesatOrientation;

    // Integrate orientation
    state.cubesatOrientation += qDot * simDeltaTime;
    state.cubesatOrientation = glm::normalize(state.cubesatOrientation);
}

