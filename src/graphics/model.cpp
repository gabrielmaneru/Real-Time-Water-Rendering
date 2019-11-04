/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	model.cpp
Purpose: Model manager
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

#include "model.h"
#include "gl_error.h"
#include "graphics/renderer.h"
#include <platform/editor.h>
#include <utils/math_utils.h>
#include <scene/scene_object.h>
#include <platform/window_manager.h>
#include <platform/window.h>

std::vector<Material*> Model::m_def_materials{};

mat4 to_glm(aiMatrix4x4 m)
{
	// Transform Column mayor/Row mayor
	return glm::transpose(glm::make_mat4(&m.a1));
}


Model::Model(const std::string & path, const std::vector<std::string>& def_mats)
{
	load_obj(path);
	if (def_mats.size())
	{
		for (int i = 0; i < def_mats.size() && i < m_materials.size(); i++)
			for (auto pMat : m_def_materials)
				if (pMat->m_name == def_mats[i])
					m_materials[i] = Material(*pMat);
	}
	m_name = path.substr(path.find_last_of('/') + 1, path.find_last_of('.') - path.find_last_of('/') - 1);
}

Model::~Model()
{
	// Clean Memory
	for (auto pM : m_meshes)
		delete pM;
	m_meshes.clear();

	for (auto pB : m_bones)
		delete pB;
	m_meshes.clear();

	for (auto pA : m_animations)
		delete pA;
	m_animations.clear();
}

void Model::draw(Shader_Program * shader, animator * m_animator, bool use_mat) const
{
	// If it has bones
	if (m_bones.size())
	{
		// Update Final Transformations of Bones
		update(m_hierarchy,m_animator, mat4(1.0f));

		// Load Bone transformations
		for (size_t i = 0; i < m_bones.size(); i++)
		{
			std::string call("bones[" + std::to_string(i) + "]");
			shader->set_uniform(call.c_str(), m_bones[i]->m_final_transform);

			if (shader == renderer->color_shader)
			{
				call= "prev_bones[" + std::to_string(i) + "]";
				shader->set_uniform(call.c_str(), m_bones[i]->m_prev_transform);
			}
		}
	}

	// Loop each mesh
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		if (use_mat)
		{
			// Load Material (hardcoded)
			if(m_meshes[i]->m_material_idx == default_material
			|| m_materials[m_meshes[i]->m_material_idx].m_name == "(null)")
				m_def_materials[0]->set_uniform(shader);
			else
				m_materials[m_meshes[i]->m_material_idx].set_uniform(shader);
		}

		// Draw it
		m_meshes[i]->draw(shader);
	}
}

void Model::load_obj(const std::string & path)
{
	Assimp::Importer importer{};
	const aiScene* scn = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs);

	if (!scn)
		throw std::string("Mesh not found: ") + path;
	if (scn->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		throw std::string("Mesh couldn't be loaded: ") + path;
	if (!scn->mRootNode)
		throw std::string("Mesh empty: ") + path;

	// Process Hierarchy
	m_hierarchy = new node(nullptr);
	processNode(scn->mRootNode, m_hierarchy, scn);

	// Link Bones to Animnodes
	for (auto b : m_bones)
		m_hierarchy->Find(b->m_name)->m_bones.push_back(b);

	// Process Animation
	if (scn->HasAnimations())
		processAnimations(scn);
}

void Model::processNode(aiNode * node_, node * parent, const aiScene * scene)
{
	// Store data
	parent->m_name = { node_->mName.data };
	parent->m_transformation = to_glm(node_->mTransformation);

	// Add Meshes
	for (size_t i = 0; i < node_->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node_->mMeshes[i]];
		m_meshes.push_back(processMesh(mesh, scene));
	}

	// Continue down the hierarchy
	for (size_t i = 0; i < node_->mNumChildren; i++)
	{
		auto child = new node(parent);
		parent->m_children.push_back(child);
		processNode(node_->mChildren[i], child, scene);
	}
}

void Model::processAnimations(const aiScene * scene)
{
	// Loop animations
	for (size_t i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* ai_anim = scene->mAnimations[i];
		animation* anim = new animation;
		
		anim->m_duration = ai_anim->mDuration;
		anim->m_tick_per_second = ai_anim->mTicksPerSecond;

		// Copy channels
		for (size_t i = 0; i < ai_anim->mNumChannels; i++)
		{
			aiNodeAnim* ai_channel = ai_anim->mChannels[i];
			std::string name = ai_channel->mNodeName.data;
			channel& c = anim->m_channels[name];

			// Add Positions
			c.m_key_position.resize(ai_channel->mNumPositionKeys);
			for (size_t i = 0; i < ai_channel->mNumPositionKeys; i++)
			{
				const aiVectorKey& key = ai_channel->mPositionKeys[i];
				c.m_key_position[i] = { glm::make_vec3(&key.mValue.x), key.mTime };
			}

			// Add Rotations
			c.m_key_rotation.resize(ai_channel->mNumRotationKeys);
			for (size_t i = 0; i < ai_channel->mNumRotationKeys; i++)
			{
				const aiQuatKey& key = ai_channel->mRotationKeys[i];
				c.m_key_rotation[i] = { quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z), key.mTime };
			}

			// Add Scales
			c.m_key_scaling.resize(ai_channel->mNumScalingKeys);
			for (size_t i = 0; i < ai_channel->mNumScalingKeys; i++)
			{
				const aiVectorKey& key = ai_channel->mScalingKeys[i];
				c.m_key_scaling[i] = { glm::make_vec3(&key.mValue.x), key.mTime };
			}
		}

		// Add new animations
		m_animations.push_back(anim);
	}
}

Mesh* Model::processMesh(aiMesh * mesh, const aiScene * scene)
{
	// Create the mesh
	Mesh* m_mesh = new Mesh(mesh->mNumVertices);
	auto quad = mesh->mPrimitiveTypes & aiPrimitiveType::aiPrimitiveType_POLYGON;
	m_mesh->m_primitive = (quad > 0) ? Mesh::e_prim::quad : Mesh::e_prim::tri;

	// Load the vertex buffer
	VertexBuffer& vertices = m_mesh->m_vertices;
	// Vertex
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		// Set position
		vertices.position[i].x = mesh->mVertices[i].x;
		vertices.position[i].y = mesh->mVertices[i].y;
		vertices.position[i].z = mesh->mVertices[i].z;

		// Set normal
		if (mesh->HasNormals())
		{
			vertices.normal[i].x = mesh->mNormals[i].x;
			vertices.normal[i].y = mesh->mNormals[i].y;
			vertices.normal[i].z = mesh->mNormals[i].z;
		}

		// Set tangents and bitangents
		if (mesh->HasTangentsAndBitangents())
		{
			vertices.tangent[i].x = mesh->mTangents[i].x;
			vertices.tangent[i].y = mesh->mTangents[i].y;
			vertices.tangent[i].z = mesh->mTangents[i].z;
			vertices.bitangent[i].x = mesh->mBitangents[i].x;
			vertices.bitangent[i].y = mesh->mBitangents[i].y;
			vertices.bitangent[i].z = mesh->mBitangents[i].z;
		}

		// Texture coordinates
		if (mesh->HasTextureCoords(0))
		{
			vertices.uv[i].x = mesh->mTextureCoords[0][i].x;
			vertices.uv[i].y = mesh->mTextureCoords[0][i].y;
		}
		else 
			vertices.uv[i] = glm::vec2(0.0f);
	}

	// Index
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
			m_mesh->m_indices.push_back(face.mIndices[j]);
	}

	// Bones
	for (size_t b = 0; b < mesh->mNumBones; b++)
	{
		const aiBone* bone = mesh->mBones[b];
		std::string bone_name = bone->mName.data;

		// Link id with bonemap
		int bone_id;
		auto it = m_bone_mapping.find(bone_name);
		if (it == m_bone_mapping.end())
		{
			bone_id = (int)m_bones.size();
			m_bone_mapping[bone_name] = bone_id;

			bone_data* bdata = new bone_data{};
			bdata->m_offset = to_glm(bone->mOffsetMatrix);
			bdata->m_name = bone_name;
			m_bones.push_back(bdata);
		}
		else
			bone_id = m_bone_mapping[bone_name];
		
		// Apply weights
		for (size_t w = 0; w < bone->mNumWeights; w++)
		{
			unsigned vtx_id = bone->mWeights[w].mVertexId;
			float weight = bone->mWeights[w].mWeight;
			for (vec4::length_type i = 0; i < 4u; i++)
			{
				if (vertices.bones[vtx_id][i] == -1)
				{
					vertices.bones[vtx_id][i] = bone_id;
					vertices.wbones[vtx_id][i] = weight;
					break;
				}
			}
		}
	}

	// Material
	m_mesh->m_material_idx = { default_material };
	if (scene->HasMaterials() && mesh->mMaterialIndex >= 0)
		m_mesh->m_material_idx = processMaterial(scene->mMaterials[mesh->mMaterialIndex]);
	m_mesh->load();
	return m_mesh;
}

int Model::processMaterial(aiMaterial * material)
{
	// Get Material Name
	aiString name_;
	material->Get(AI_MATKEY_NAME, name_);
	std::string name{ name_.data };

	// Check Default Material
	if (name == "DefaultMaterial")
		return default_material;

	// Search among already existing materials
	for (int i = 0; i < (int)m_materials.size(); i++)
		if (m_materials[i].m_name == name)
			return i;

	// Add new Material
	m_materials.push_back({});
	int idx = (int)m_materials.size() - 1;
	Material& mat = m_materials[idx];
	mat.m_name = name;

	// Extract material keys
	aiColor3D color;
	material->Get(AI_MATKEY_COLOR_AMBIENT, color);
	mat.m_ambient = color.r;
	material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	mat.m_albedo.r = color.r;
	mat.m_albedo.g = color.g;
	mat.m_albedo.b = color.b;
	material->Get(AI_MATKEY_COLOR_SPECULAR, color);
	mat.m_metallic.r = color.r;
	mat.m_metallic.g = color.g;
	mat.m_metallic.b = color.b;
	
	mat.m_albedo_txt = loadMaterialTexture(material, aiTextureType_DIFFUSE);
	mat.m_metallic_txt = loadMaterialTexture(material, aiTextureType_AMBIENT);
	mat.m_roughness_txt = loadMaterialTexture(material, aiTextureType_SHININESS);
	mat.m_normal_txt = loadMaterialTexture(material, aiTextureType_HEIGHT);
	if(mat.m_normal_txt.m_id == 0)
		mat.m_normal_txt = loadMaterialTexture(material, aiTextureType_NORMALS);
	return idx;
}

Texture Model::loadMaterialTexture(aiMaterial * material, aiTextureType type)
{
	if (material->GetTextureCount(type) == 0)
		return Texture{};

	aiString str;
	material->GetTexture(type, 0, &str);

	Texture texture;
	texture.m_path = Texture::filter_name(str.C_Str());
	texture.loadFromFile(texture.m_path.c_str(), type == aiTextureType_DIFFUSE);
	return texture;
}

void Model::update(node * node_, animator * m_animator, mat4 parent) const
{
	mat4 node_transformation = node_->m_transformation;
	if (m_animator && m_animator->m_current_animation != -1 && m_animations.size() > 0)
	{
		animation* anim = m_animations[m_animator->m_current_animation];

		auto it = anim->m_channels.find(node_->m_name);
		if (it != anim->m_channels.end())
		{
			const channel& c = it->second;
			vec3 scaling  =  c.lerp_scaling(m_animator->m_time);
			quat rotation = c.lerp_rotation(m_animator->m_time);
			vec3 position = c.lerp_position(m_animator->m_time);

			mat4 scl_mat = glm::scale(mat4(1.0f), vec3{ scaling });
			mat4 rot_mat = glm::mat4_cast(rotation);
			mat4 pos_mat = glm::translate(mat4(1.0f), position);
			node_transformation = pos_mat * rot_mat * scl_mat;
		}
	}

	node_transformation = parent * node_transformation;

	for (auto b : node_->m_bones)
	{
		b->m_prev_transform = b->m_final_transform;
		b->m_final_transform = node_transformation * b->m_offset;
	}

	for (auto c : node_->m_children)
		update(c, m_animator, node_transformation);
}

node::node(node * parent)
	: m_parent(parent) {}

node::~node()
{
	for (auto c : m_children)
		delete c;
	m_children.clear();
}

node * node::Find(std::string name)
{
	if (m_name == name)
		return this;

	for (auto c : m_children)
	{
		node* res = c->Find(name);
		if (res != nullptr)
			return res;
	}
	return nullptr;
}

vec3 channel::lerp_position(double time)const
{
	assert(m_key_position.size() > 0);
	if (m_key_position.size() == 1)
		return m_key_position[0].first;

	for (size_t i = 0; i < m_key_position.size() - 1; i++)
		if (time < m_key_position[i + 1].second)
			return map(time, m_key_position[i].second, m_key_position[i + 1].second,
				m_key_position[i].first, m_key_position[i + 1].first);
	return{};
}

quat channel::lerp_rotation(double time)const
{
	assert(m_key_rotation.size() > 0);
	if (m_key_rotation.size() == 1)
		return m_key_rotation[0].first;

	for (size_t i = 0; i < m_key_rotation.size() - 1; i++)
		if (time < m_key_rotation[i + 1].second)
			return map(time, m_key_rotation[i].second, m_key_rotation[i + 1].second,
				m_key_rotation[i].first, m_key_rotation[i + 1].first);
	return{};
}

vec3 channel::lerp_scaling(double time)const
{
	assert(m_key_scaling.size() > 0);
	if (m_key_scaling.size() == 1)
		return m_key_scaling[0].first;

	for (size_t i = 0; i < m_key_scaling.size() - 1; i++)
		if (time < m_key_scaling[i + 1].second)
			return map(time, m_key_scaling[i].second, m_key_scaling[i + 1].second,
				m_key_scaling[i].first, m_key_scaling[i + 1].first);
	return{};
}
