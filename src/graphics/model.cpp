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
#include <platform/editor.h>

const Material Model::m_def_material
{
	"default",
	vec3(1.0),
	vec3(0.0, 1.0, 0.0),
	vec3(1.0, 1.0, 1.0),
	10.f,
	{},
	{},
	{}
};
mat4 to_glm(aiMatrix4x4 m)
{
	return mat4{
		m.a1, m.a2, m.a3, m.a4,
		m.b1, m.b2, m.b3, m.b4,
		m.c1, m.c2, m.c3, m.c4,
		m.d1, m.d2, m.d3, m.d4
	};
}

Model::Model(const std::string & path)
{
	load_obj(path);
	m_name = path.substr(path.find_last_of('/') + 1, path.find_last_of('.') - path.find_last_of('/') - 1);
}

void Model::draw(Shader_Program * shader, bool use_mat)const
{
	for (auto& mesh : m_meshes)
	{
		if (use_mat)
		{
			if (mesh->m_material_idx == default_material)
				m_def_material.set_uniform(shader);
			else
				m_materials[mesh->m_material_idx].set_uniform(shader);
		}
		mesh->draw(shader);
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

	m_hierarchy = new Node(nullptr);
	processNode(scn->mRootNode, m_hierarchy, scn);
}

void Model::processNode(aiNode * node, Node * parent, const aiScene * scene)
{
	parent->m_name = { node->mName.data };
	parent->m_transformation = to_glm(node->mTransformation);
	std::string name{ node->mName.data };
	for (size_t i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_meshes.push_back(processMesh(mesh, scene));
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		auto child = new Node(parent);
		parent->m_children.push_back(child);
		processNode(node->mChildren[i], child, scene);
	}
}

Mesh* Model::processMesh(aiMesh * mesh, const aiScene * scene)
{
	Mesh* m_mesh = new Mesh(mesh->mNumVertices);
	auto quad = mesh->mPrimitiveTypes & aiPrimitiveType::aiPrimitiveType_POLYGON;
	m_mesh->m_primitive = (quad > 0) ? Mesh::e_prim::quad : Mesh::e_prim::tri;

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
		m_mesh->m_bone_mapping[bone_name] = b;

		BoneData bdata;
		bdata.m_offset = to_glm(bone->mOffsetMatrix);
		m_mesh->m_bones.push_back(bdata);

		for (size_t w = 0; w < bone->mNumWeights; w++)
		{
			unsigned id = bone->mWeights[w].mVertexId;
			float weight = bone->mWeights[w].mWeight;
			for (vec4::length_type i = 0; i < 4u; i++)
			{
				if (vertices.bones[id][i] == -1)
				{
					vertices.bones[id][i] = (int)b;
					vertices.wbones[id][i] = weight;
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
	mat.m_ambient.r = color.r;
	mat.m_ambient.g = color.g;
	mat.m_ambient.b = color.b;
	material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	mat.m_diffuse.r = color.r;
	mat.m_diffuse.g = color.g;
	mat.m_diffuse.b = color.b;
	material->Get(AI_MATKEY_COLOR_SPECULAR, color);
	mat.m_specular.r = color.r;
	mat.m_specular.g = color.g;
	mat.m_specular.b = color.b;

	material->Get(AI_MATKEY_SHININESS, mat.m_shininess);
	
	mat.m_diffuse_txt = loadMaterialTexture(material, aiTextureType_DIFFUSE);
	mat.m_specular_txt = loadMaterialTexture(material, aiTextureType_SPECULAR);
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

	std::string name = str.C_Str();
	size_t start = name.find_last_of('/') + 1;
	if (start > name.size()) start = 0;
	if(start == 0)
	{
		start = name.find_last_of('\\') + 1;
		if (start > name.size()) start = 0;
	}
	name = "./data/textures/" + name.substr(start);

	Texture texture;
	texture.loadFromFile(name.c_str(), type == Texture::e_texture_type::DIFFUSE);
	texture.m_type = static_cast<Texture::e_texture_type>(type);
	texture.m_path = str.C_Str();
	return texture;
}

Node::Node(Node * parent)
	: m_parent(parent) {}

Node::~Node()
{
	for (auto c : m_children)
		delete c;
	m_children.clear();
}
