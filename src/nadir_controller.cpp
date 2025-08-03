#define GLM_ENABLE_EXPERIMENTAL
#include "nadir_controller.h"
#include "attitude.h"
#include "constants.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <cmath>

constexpr glm::vec3 inertiaDiag(0.0008f, 0.0010f, 0.0009f); 
constexpr float wn   = 0.3f;
constexpr float zeta = 1.0f;   

const glm::vec3 Kp = inertiaDiag * (wn * wn);
const glm::vec3 Kd = inertiaDiag * (2.0f * zeta * wn);

constexpr float ANGLE_DEADBAND_DEG = 1.0f;
constexpr float RATE_DEADBAND      = 0.0005f;

glm::vec3 computeNadirTorque(const SimulationState& state)
{
    glm::vec3 r = state.cubesatPos;
    glm::vec3 v = state.cubesatVel;

    // +Z → Earth
    glm::vec3 z_dir = glm::normalize(-r);
    glm::vec3 h = glm::normalize(glm::cross(r, v));
    glm::vec3 x_dir = glm::normalize(glm::cross(h, z_dir));
    glm::vec3 y_dir = glm::cross(z_dir, x_dir);

    glm::mat3 R_desired(x_dir, y_dir, z_dir);
    glm::quat q_desired = glm::normalize(glm::quat_cast(R_desired));

    glm::quat q_err = q_desired * glm::inverse(state.cubesatOrientation);
    if (q_err.w < 0.0f) q_err = -q_err;
    q_err = glm::normalize(q_err);

    glm::vec3 errorAxis = glm::axis(q_err);
    float errorAngle = glm::angle(q_err);
    if (glm::any(glm::isnan(errorAxis))) { errorAxis = glm::vec3(0.0f); errorAngle = 0.0f; }

    // Orbital rate
    float mu = Physics::G * Physics::EARTH_MASS;
    float r_unscaled = glm::length(r); // FIX FIX FIX
    float orbitalRate = std::sqrt(mu / (r_unscaled * r_unscaled * r_unscaled));
    glm::vec3 desiredAngVel = R_desired * glm::vec3(0.0f, orbitalRate, 0.0f);

    glm::vec3 angVelError = state.cubesatAngularVel - desiredAngVel;
 
    // If pointing error is large, focus only on attitude correction
    glm::vec3 proportional = Kp * errorAngle * errorAxis;
    glm::vec3 derivative = (glm::degrees(errorAngle) > 20.0f)
                            ? inertiaDiag * -state.cubesatAngularVel
                            : Kd * angVelError;

    glm::vec3 controlTorque = -(proportional + derivative);

    constexpr float TORQUE_LIMIT = 0.002f;
    controlTorque = glm::clamp(controlTorque, -TORQUE_LIMIT, TORQUE_LIMIT);

    return controlTorque;
}

