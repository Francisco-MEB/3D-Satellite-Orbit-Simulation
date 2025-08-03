#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <cstddef>

#include "sphere.h"

Sphere::Sphere(unsigned int sectorCount, unsigned int stackCount, float radius)
{
    std::vector<float> vertices {};
    std::vector<unsigned int> indices {};
    createSphere(vertices, indices, sectorCount, stackCount, radius);
    indexCount = static_cast<unsigned int>(indices.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

Sphere::~Sphere()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Sphere::draw() const
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Sphere::createSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices,
                          unsigned int sectorCount, unsigned int stackCount, float radius)
{
    for (unsigned int i = 0; i < stackCount; ++i)
    {
        float stackAngle { MathConstants::PI / 2.0f - i * (MathConstants::PI / stackCount) };
        float xy { radius * cosf(stackAngle) };
        float z { radius * sinf(stackAngle) };

        for (unsigned int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle { j * (2.0f * MathConstants::PI / sectorCount) };

            float x { xy * cosf(sectorAngle) };
            float y { xy * sinf(sectorAngle) };

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal
            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);

            // Tex coords
            vertices.push_back(static_cast<float>(j) / sectorCount);
            vertices.push_back(static_cast<float>(i) / stackCount);
        }
    }

    for (unsigned int i = 0; i < stackCount; ++i)
    {
        for (unsigned int j = 0; j < sectorCount; ++j)
        {
            unsigned int first { i * (sectorCount + 1) + j };
            unsigned int second { first + sectorCount + 1 };

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}
