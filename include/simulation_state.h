#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

#include <glm/glm.hpp>

#include "camera.h"
#include "constants.h"
#include "reaction_wheel_system.h"

enum class CameraMode { FREE, FOLLOW, ONBOARD };

struct SimulationState
{ 
    float deltaTime { 0.0f };
    float lastFrame { 0.0f };

    glm::vec3 lightPos { 
       glm::vec3(100.0f, 0.0f, 100.0f) 
    };

    glm::vec3 cubesatPos { 
       glm::vec3(0.0f, 0.0f, Physics::EARTH_RADIUS_SCALED + Physics::ORBIT_ALTITUDE_SCALED) 
    };

    glm::vec3 cubesatVel { glm::vec3(0.0f) };

    glm::quat cubesatOrientation { glm::quat(1.0f, 0.0f, 0.0f, 0.0f) };
      
    glm::vec3 cubesatAngularVel { glm::vec3(0.0f) };

    ReactionWheelSystem wheels;

    CameraMode cameraMode { CameraMode::FREE };

    float simElapsedTime { 0.0f };
};

#endif
