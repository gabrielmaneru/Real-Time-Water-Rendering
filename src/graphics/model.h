#pragma once
#include "shader_program.h"
#include "mesh.h"
#include "texture.h"
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	Model() = default;
	Model(const std::string&);
	void draw(Shader_Program *)const;
private:
	void load_obj(const std::string&);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType type, Texture::e_texture_type textureType);
	unsigned int textureFromFile(const char* str, Texture::e_texture_type type);

	std::vector<Mesh> m_meshes;
	std::vector<Texture> m_textures;
};