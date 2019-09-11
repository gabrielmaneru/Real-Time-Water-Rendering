#pragma once
#include "shader_program.h"
#include "mesh.h"
#include "texture.h"
#include "material.h"
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	Model(const std::string&);
	void draw(Shader_Program *)const;
private:
	void load_obj(const std::string&);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene);
	int processMaterial(aiMaterial * material);
	Texture loadMaterialTexture(aiMaterial *material, aiTextureType type);

	std::vector<Mesh*> m_meshes;
	std::vector<Texture> m_textures;
	std::vector<Material> m_materials;
};