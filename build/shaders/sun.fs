#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sunMap;

void main()
{
    vec3 color = texture(sunMap, TexCoords).rgb;
    FragColor = vec4(color * 2.0, 1.0);
}
