#ifndef SPHERE_H
#define SPHERE_H

#include <glad/glad.h>

#include <vector>

namespace MathConstants
{
    inline constexpr float PI { 3.14159265359f };
}

class Sphere
{
public:
    unsigned int VAO {}, VBO {}, EBO {};
    unsigned int indexCount {};

    Sphere(unsigned int sectorCount = 64, unsigned int stackCount = 64, float radius = 1.0f);
    ~Sphere();
    void draw() const;

private:
    void createSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices,
                      unsigned int sectorCount, unsigned int stackCount, float radius);
};

#endif
