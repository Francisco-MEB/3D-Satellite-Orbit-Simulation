#version 330 core
struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D topDiffuse;
    sampler2D bottomDiffuse;
    float shininess;
};

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
flat in int FaceID;

uniform Light light;
uniform Material material;
uniform vec3 viewPos;

void main()
{
    vec3 baseColor;

    if (FaceID == 1)
        baseColor = texture(material.topDiffuse, TexCoords).rgb;
    else if (FaceID == 2)
        baseColor = texture(material.bottomDiffuse, TexCoords).rgb;
    else
        baseColor = texture(material.diffuse, TexCoords).rgb;

    vec3 ambient = light.ambient * baseColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * baseColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}

