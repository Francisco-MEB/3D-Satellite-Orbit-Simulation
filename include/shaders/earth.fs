#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D earthMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 baseColor = texture(earthMap, TexCoords).rgb; 

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient = 0.2 * baseColor;
    vec3 diffuse = 0.6 * diff * baseColor;
    vec3 specular = 0.2 * spec * vec3(0.05);

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
