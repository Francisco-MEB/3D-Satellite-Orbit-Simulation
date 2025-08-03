#include <glad/glad.h>
#include "cube.h"
#include "geometric_data.h"

CubeSat::CubeSat()
    : textureID { 0 }
{
   glGenVertexArrays(1, &VAO);
   glGenBuffers(1, &VBO);
   glGenBuffers(1, &EBO);

   glBindVertexArray(VAO);

   glBindBuffer(GL_ARRAY_BUFFER, VBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(CubeSatConsts::vertices), CubeSatConsts::vertices, GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeSatConsts::indices), CubeSatConsts::indices, GL_STATIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);
   
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);

   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
   glEnableVertexAttribArray(2);

   glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));
   glEnableVertexAttribArray(3);

   glBindVertexArray(0);
}

CubeSat::~CubeSat()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void CubeSat::draw() const
{
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
