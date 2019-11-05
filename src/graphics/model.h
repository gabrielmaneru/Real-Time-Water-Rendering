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
#include "shader_program.h"
#include <string>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
struct bone_data;
struct animator;
struct node
{
	node(node*);
	~node();
	node* Find(std::string);

	node* m_parent;
	std::vector<node*> m_children;
	std::vector<bone_data*> m_bones;
	std::string m_name;
	mat4 m_transformation;
};

struct bone_data
{
	mat4 m_offset;
	mutable mat4 m_final_transform;
	mutable mat4 m_prev_transform{1.0f};
	std::string m_name;
};

using key_vec3 = std::pair<vec3, double>;
using key_quat = std::pair<quat, double>;
struct channel
{
	vec3 lerp_position(double time)const;
	quat lerp_rotation(double time)const;
	vec3 lerp_scaling (double time)const;

	std::vector<key_vec3> m_key_position;
	std::vector<key_quat> m_key_rotation;
	std::vector<key_vec3> m_key_scaling;
};

struct animation
{
	double m_duration;
	double m_tick_per_second;
	std::map<std::string,channel> m_channels;
};

class Model
{
public:
	Model(const std::string& path, const std::vector<std::string>& def_mats = {}, const std::vector<size_t>& skip_meshes = {}, const std::vector<std::pair<size_t,size_t>>& break_animation = {});
	~Model();
	void draw(Shader_Program *, animator* m_animator, bool use_mat = true)const;

	std::string m_name;
	std::vector<Mesh*> m_meshes;

	static std::vector<Material*> m_def_materials;

private:
	void load_obj(const std::string&);
	void processNode(aiNode* node_, node* parent, const aiScene* scene);
	void processAnimations(const aiScene* scene);
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene);
	int processMaterial(aiMaterial * material);
	Texture loadMaterialTexture(aiMaterial *material, aiTextureType type);
	void update(node* node_, animator* m_animator, mat4 parent)const;

	std::vector<Texture> m_textures;
	std::vector<Material> m_materials;
	node* m_hierarchy;
	std::vector<bone_data*> m_bones;
	std::map<std::string, int> m_bone_mapping;
	std::vector<animation*> m_animations;
	friend class scene_object;
};