#ifndef REACTION_WHEEL_SYSTEM_H
#define REACTION_WHEEL_SYSTEM_H

#include "reaction_wheel.h"
#include <glm/glm.hpp>
#include <array>

inline constexpr int numWheels { 3 };

class ReactionWheelSystem
{
public:
    ReactionWheelSystem();

    void applyTorqueCommands(const glm::vec3& torques, float dt);
    void update(float dt);
    glm::vec3 computeReactionTorque(float dt);

    glm::vec3 getTotalMomentum() const;
    std::array<float, 3> getSpeeds() const;

    glm::vec3 getTorque() const;
    
    float getWheelAngularVelocity(int idx) const;

    glm::vec3 getWheelAxis(int idx) const;

private:
    std::array<ReactionWheel, numWheels> m_wheels;
    glm::vec3 m_lastReactionTorque { 0.0f };
};

#endif
