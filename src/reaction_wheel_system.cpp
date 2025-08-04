#include "reaction_wheel_system.h"

ReactionWheelSystem::ReactionWheelSystem()
    : m_wheels {
        ReactionWheel(glm::vec3(1, 0, 0), 0.01f),
        ReactionWheel(glm::vec3(0, 1, 0), 0.01f),
        ReactionWheel(glm::vec3(0, 0, 1), 0.01f)
    }
{
}

void ReactionWheelSystem::applyTorqueCommands(const glm::vec3& torques, float dt)
{
    for (int i = 0; i < numWheels; ++i)
        m_wheels[i].applyTorque(torques[i], dt);
}

void ReactionWheelSystem::update(float dt)
{
    for (auto& wheel : m_wheels)
        wheel.update(dt);
}

glm::vec3 ReactionWheelSystem::getTotalMomentum() const
{
    glm::vec3 L(0.0f);
    for (const auto& wheel : m_wheels)
        L += wheel.getAngularMomentum();

    return L;
}

glm::vec3 ReactionWheelSystem::computeReactionTorque(float dt)
{
    static glm::vec3 prevMomentum(0.0f);
    glm::vec3 currentMomentum = getTotalMomentum();
    m_lastReactionTorque = -(currentMomentum - prevMomentum) / dt; // Store
    prevMomentum = currentMomentum;

    return m_lastReactionTorque;
}

std::array<float, numWheels> ReactionWheelSystem::getSpeeds() const
{
    std::array<float, numWheels> res {};
    for (int i = 0; i < numWheels; ++i)
        res[i] = m_wheels[i].getAngularVelocity();

    return res;
}

glm::vec3 ReactionWheelSystem::getTorque() const {
    return m_lastReactionTorque; // Return stored value
}

float ReactionWheelSystem::getWheelAngularVelocity(int idx) const {
    return m_wheels[idx].getAngularVelocity();
}

glm::vec3 ReactionWheelSystem::getWheelAxis(int idx) const {
    return m_wheels[idx].getAxis();
}
