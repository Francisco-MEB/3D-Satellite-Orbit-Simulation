#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "constants.h"

inline constexpr int FONT_SIZE { 24 };

inline constexpr float SECS_IN_DAY { 86400.0f };

inline constexpr float MAX_DELTA_TIME { 0.01f }; // 100 FPS
                                                 //
// makes Earth radius 30 OpenGL units
inline constexpr float SCALE_FACTOR { 1.0f / 212367.0f };

namespace Physics
{
    inline constexpr float G { 6.67430e-11f }; // m^3 / (kg s^2)
    inline constexpr float EARTH_MASS { 5.972e24f }; // kg
    inline constexpr float CUBESAT_MASS { 1.33f }; // kg (1U)
                                                   
    inline constexpr float EARTH_RADIUS { 6.371e6f }; // meters
    inline constexpr float EARTH_RADIUS_SCALED { 6.371e6f * SCALE_FACTOR };

    inline constexpr float ORBIT_ALTITUDE { 4.0e5f }; // 400 km 
    inline constexpr float ORBIT_ALTITUDE_SCALED { 4.0e5f * SCALE_FACTOR };
};

namespace RenderSettings
{
    inline constexpr int SPHERE_SECTOR_COUNT { 64 };
    inline constexpr int SPHERE_STACK_COUNT { 64 };
}

#endif
