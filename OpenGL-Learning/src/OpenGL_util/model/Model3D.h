#pragma once

#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model3D
{
private:
	std::vector<Mesh> m_Meshes;
	std::string m_Directory;

	std::vector<Texture*> m_LoadedTextures;

	void Load(const std::string& path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);

public:
	Model3D(const char* path);
	~Model3D();

	void Draw(Shader& shader);
};

