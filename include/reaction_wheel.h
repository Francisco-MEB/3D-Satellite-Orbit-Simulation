#ifndef REACTION_WHEEL_H
#define REACTION_WHEEL_H

#include <glm/glm.hpp>

class ReactionWheel
{
public:
    ReactionWheel(glm::vec3 axis, float inertia);

    void applyTorque(float torque, float dt);
    void update(float dt);

    float getAngularVelocity() const;
    glm::vec3 getAngularMomentum() const;

    glm::vec3 getAxis() const;
private:
    glm::vec3 m_axis;
    float m_inertia;
    float m_angularVel;
};

#endif
