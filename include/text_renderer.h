#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <glm/glm.hpp>

#include <map>
#include <string>

#include "shader_s.h"

struct Character {
    unsigned int TextureID;
    glm::ivec2 Size;         
    glm::ivec2 Bearing;     
    unsigned int Advance; 
};

class TextRenderer {
public:
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;

    TextRenderer(unsigned int width, unsigned int height); 
    void Load(std::string font, unsigned int fontSize);   
    void RenderText(Shader &shader, std::string_view text, float x, float y, float scale, glm::vec3 color);

private:
    unsigned int m_width, m_height;
};

#endif

