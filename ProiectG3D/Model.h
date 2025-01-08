#pragma once

#include <vector>
#include <iostream>
#include "Texture.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model
{
public:

    vector<Texture> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;
    std::unordered_map<std::string, glm::mat4> nodeTransforms;
    bool isAnimated = false;


	Model() = default;

    Model(string const& path, bool bSmoothNormals, bool gamma = false);

    virtual void Draw(Shader& shader);

    void setNodeTransforms(const std::string& nodeName, glm::mat4 transform);
    void setIsAnimated(bool animated);

private:
    void loadModel(string const& path, bool bSmoothNormals);

    void processNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(std::string nodeName, aiMesh* mesh, const aiScene* scene);

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

};

