#define GLM_ENABLE_EXPERIMENTAL
#include "nadir_controller.h"
#include "attitude.h"
#include "constants.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <cmath>

// Control tuning
constexpr float wn   = 0.15f;   // natural frequency (rad/s) – faster response
constexpr float zeta = 0.9f;   // damping ratio (close to critical)

// CubeSat inertia (diagonal)
constexpr glm::vec3 inertiaDiag(0.0008f, 0.0010f, 0.0009f);

// Gains
const glm::vec3 Kp = inertiaDiag * (wn * wn);
const glm::vec3 Kd = inertiaDiag * (2.0f * zeta * wn);

glm::vec3 computeNadirTorque(const SimulationState& state)
{
    // 1️⃣ Compute desired orientation (nadir-pointing frame)
    glm::vec3 z = -glm::normalize(state.cubesatPos);  // nadir = forward
    glm::vec3 orbitNormal = glm::normalize(glm::cross(state.cubesatPos, state.cubesatVel));
    glm::vec3 x = glm::normalize(glm::cross(orbitNormal, z)); // right
    glm::vec3 y = glm::cross(z, x); // up

    glm::mat3 R_desired(x, y, z); 
    glm::quat q_desired = glm::quat_cast(R_desired);

    // 2️⃣ Compute quaternion error
    glm::quat q_err = q_desired * glm::inverse(state.cubesatOrientation);
    q_err = glm::normalize(q_err);

    glm::vec3 errorAxis = glm::axis(q_err);
    float errorAngle = glm::angle(q_err);

    if (glm::any(glm::isnan(errorAxis))) {
        errorAxis = glm::vec3(0.0f);
        errorAngle = 0.0f;
    }

    // 3️⃣ Compute feedforward orbital angular velocity
    float mu = Physics::G * Physics::EARTH_MASS;
    float r_unscaled = glm::length(state.cubesatPos) / SCALE_FACTOR;
    float orbitalRate = std::sqrt(mu / (r_unscaled * r_unscaled * r_unscaled));
    glm::vec3 desiredAngVel = orbitalRate * orbitNormal;

    // 4️⃣ PD control terms
    glm::vec3 proportional = glm::vec3(
        Kp.x * errorAngle * errorAxis.x,
        Kp.y * errorAngle * errorAxis.y,
        Kp.z * errorAngle * errorAxis.z
    );

    glm::vec3 angVelError = state.cubesatAngularVel - desiredAngVel;
    glm::vec3 derivative = glm::vec3(
        Kd.x * angVelError.x,
        Kd.y * angVelError.y,
        Kd.z * angVelError.z
    );

    glm::vec3 controlTorque = -(proportional + derivative);

    // 5️⃣ Debug info
    static float lastPrint = 0.0f;
    if (state.lastFrame - lastPrint > 1.0f) { // print every 1s
        std::cout << "Error angle (deg): " << glm::degrees(errorAngle)
                  << " | Desired w: (" << desiredAngVel.x << ", " << desiredAngVel.y << ", " << desiredAngVel.z << ")"
                  << " | Actual w: (" << state.cubesatAngularVel.x << ", " << state.cubesatAngularVel.y << ", " << state.cubesatAngularVel.z << ")"
                  << " | Torque: (" << controlTorque.x << ", " << controlTorque.y << ", " << controlTorque.z << ")"
                  << std::endl;
        lastPrint = state.lastFrame;
    }

    return controlTorque;
}

