#include "reaction_wheel.h"
#include <glm/glm.hpp>

ReactionWheel::ReactionWheel(glm::vec3 axis, float inertia)
    : m_axis(glm::normalize(axis)), m_inertia(inertia), m_angularVel(0.0f) {}

void ReactionWheel::applyTorque(float torque, float dt)
{
    constexpr float maxTorque = 0.00005f; // N·m, typical for CubeSat wheels
    float limitedTorque = glm::clamp(torque, -maxTorque, maxTorque);

    float angularAcc = limitedTorque / m_inertia;
    m_angularVel += angularAcc * dt;

    // Optional: Clamp wheel speed (rad/s)
    constexpr float maxWheelSpeed = 6000.0f * 2.0f * 3.14159f / 60.0f; // 6000 rpm
    m_angularVel = glm::clamp(m_angularVel, -maxWheelSpeed, maxWheelSpeed);
}

void ReactionWheel::update(float dt)
{
    // Optional: add friction or saturation here
    // Example:
    // m_angularVel *= 0.999f; // tiny damping
}

float ReactionWheel::getAngularVelocity() const
{
    return m_angularVel;
}

glm::vec3 ReactionWheel::getAngularMomentum() const
{
    return m_axis * (m_inertia * m_angularVel);
}

glm::vec3 ReactionWheel::getAxis() const { return m_axis; }

