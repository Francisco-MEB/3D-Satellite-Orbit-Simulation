#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 color;

void main()
{
    vec3 norm = normalize(Normal);
    float intensity = dot(norm, normalize(vec3(0.5, 0.5, 1.0)));
    intensity = clamp(intensity, 0.2, 1.0);
    FragColor = vec4(color * intensity, 1.0);
}

