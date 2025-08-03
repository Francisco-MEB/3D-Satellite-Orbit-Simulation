#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"
#include "Shader_s.h"

#include <string>
#include <vector>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

using namespace std;

class Model 
{
public:
    // model data
    vector<Texture> textures_loaded; // to avoid loading duplicates
    vector<Mesh> meshes;
    string directory;

    Model(const string &path) { loadModel(path); }
    void Draw(Shader &shader);

private:
    void loadModel(string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

#endif

