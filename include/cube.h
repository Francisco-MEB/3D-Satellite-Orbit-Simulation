#ifndef CUBE_H
#define CUBE_H

class CubeSat
{
public:
    CubeSat();
    ~CubeSat();

    void draw() const;
    void bindTexture(unsigned int texture);

    unsigned int VAO;
private:
    unsigned int VBO, EBO;
    unsigned int textureID;
};

#endif
