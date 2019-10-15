/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	model.h
Purpose: Model manager
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#pragma once
#include "shader_program.h"
#include "mesh.h"
#include "texture.h"
#include "material.h"
#include <string>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Node
{
	Node(Node*);
	~Node();

	Node* m_parent;
	std::vector<Node*> m_children;

	std::string m_name;
	mat4 m_transformation;
};

class Model
{
public:
	Model(const std::string&);
	void draw(Shader_Program *, bool use_mat = true)const;

	std::string m_name;
	std::vector<Mesh*> m_meshes;

private:
	void load_obj(const std::string&);
	void processNode(aiNode* node, Node* parent, const aiScene* scene);
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene);
	int processMaterial(aiMaterial * material);
	Texture loadMaterialTexture(aiMaterial *material, aiTextureType type);

	std::vector<Texture> m_textures;
	std::vector<Material> m_materials;
	static const Material m_def_material;
	Node* m_hierarchy;
};