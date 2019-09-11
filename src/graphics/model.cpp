#include "model.h"
#include "gl_error.h"
Model::Model(const std::string & path)
{
	load_obj(path);
}


void Model::draw(Shader_Program * shader)const
{
	for (auto& mesh : m_meshes)
	{
		if (mesh->m_material_idx >= 0)
			m_materials[mesh->m_material_idx].set_uniform(shader);
		mesh->draw(shader);
	}
}

void Model::load_obj(const std::string & path)
{
	Assimp::Importer importer{};
	const aiScene* scn = importer.ReadFile(path, aiProcess_CalcTangentSpace);

	if (!scn)
		throw std::string("Mesh not found: ") + path;
	if (scn->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		throw std::string("Mesh couldn't be loaded: ") + path;
	if (!scn->mRootNode)
		throw std::string("Mesh empty: ") + path;

	processNode(scn->mRootNode, scn);
}

void Model::processNode(aiNode * node, const aiScene * scn)
{
	for (size_t i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scn->mMeshes[node->mMeshes[i]];
		m_meshes.push_back(processMesh(mesh, scn));
	}

	for (size_t i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scn);
	}
}

Mesh* Model::processMesh(aiMesh * mesh, const aiScene * scene)
{
	VertexBuffer vertices{ mesh->mNumVertices };
	std::vector<GLuint> indices;

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
			indices.push_back(face.mIndices[j]);
	}

	// Material
	int material_idx{ -1 };
	if (scene->HasMaterials() && mesh->mMaterialIndex > 0)
		material_idx = processMaterial(scene->mMaterials[mesh->mMaterialIndex]);

	return new Mesh(vertices, indices, material_idx);
}

int Model::processMaterial(aiMaterial * material)
{
	// Get Material Name
	aiString name_;
	material->Get(AI_MATKEY_NAME, name_);
	std::string name{ name_.C_Str() };

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
	return idx;
}

Texture Model::loadMaterialTexture(aiMaterial * material, aiTextureType type)
{
	if (material->GetTextureCount(type) == 0)
		return Texture{};

	aiString str;
	material->GetTexture(type, 0, &str);

	Texture texture;
	texture.loadFromFile(str.C_Str(), type == Texture::e_texture_type::DIFFUSE);
	texture.m_type = static_cast<Texture::e_texture_type>(type);
	texture.m_path = str.C_Str();
	return texture;
}