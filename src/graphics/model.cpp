#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

Model::Model(const std::string & path)
{
	load_obj(path);
}


void Model::draw(Shader_Program * shader)const
{
	for (auto& mesh : m_meshes)
		mesh.draw(shader);
}

void Model::load_obj(const std::string & path)
{
	Assimp::Importer importer{};
	const aiScene* scn = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scn || scn->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scn->mRootNode)
		throw std::string("Mesh not loaded: ") + path;

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

Mesh Model::processMesh(aiMesh * mesh, const aiScene * scene)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	// Vertex
	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;

		// Set position
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		// Set normal
		if (mesh->HasNormals()) {
			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;
		}

		// Set tangents and bitangents
		if (mesh->HasTangentsAndBitangents()) {
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;
			vertex.bitangent.x = mesh->mBitangents[i].x;
			vertex.bitangent.y = mesh->mBitangents[i].y;
			vertex.bitangent.z = mesh->mBitangents[i].z;
		}

		// Texture coordinates
		if (mesh->HasTextureCoords(0)) {
			vertex.textureCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.textureCoords.y = mesh->mTextureCoords[0][i].y;
		}
		else {
			vertex.textureCoords = glm::vec2(0.0f);
		}

		vertices.push_back(vertex);
	}

	// Index
	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// Material
	if (mesh->mMaterialIndex >= 0) {
		// Get the material
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// Load the textures
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, Texture::e_texture_type::DIFFUSE);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> metalnessMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, Texture::e_texture_type::METALNESS);
		textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());
		
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, Texture::e_texture_type::NORMAL);
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<Texture> roughnessMaps = loadMaterialTextures(material, aiTextureType_SHININESS, Texture::e_texture_type::ROUGHNESS);
		textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial * material, aiTextureType type, Texture::e_texture_type textureType)
{
	std::vector<Texture> textures;
	for (size_t i = 0; i < material->GetTextureCount(type); i++) {
		aiString str;
		material->GetTexture(type, (unsigned int)i, &str);
		bool skipTextureLoad = false;

		// Check if the current texture has been loaded already
		for (size_t j = 0; j < m_textures.size(); j++) {
			if (std::strcmp(m_textures[j].m_path.c_str(), str.C_Str()) == 0) {
				textures.push_back(m_textures[j]);
				skipTextureLoad = true;
				break;
			}
		}

		// Textures isn't loaded yet
		if (!skipTextureLoad) {
			Texture texture;
			texture.m_id = textureFromFile(str.C_Str(), textureType);

			texture.m_type = textureType;
			texture.m_path = str.C_Str();
			textures.push_back(texture);
			m_textures.push_back(texture);
		}
	}

	return textures;
}

unsigned int Model::textureFromFile(const char * str, Texture::e_texture_type type)
{
	//stbi_set_flip_vertically_on_load(true);
	bool gamma_correction = type == Texture::e_texture_type::DIFFUSE;

	Texture texture;

	// Generate and bind texture
	glGenTextures(1, &texture.m_id);

	// Load textures
	int nrChannels;
	unsigned char* data = stbi_load((std::string(".")+str).c_str(), &texture.m_width, &texture.m_height, &nrChannels, 0);
	if (data)
	{
		GLenum format, internalFormat;
		if (nrChannels == 1) {
			format = internalFormat = GL_RED;
		}

		else if (nrChannels == 3) {
			if (gamma_correction)
				internalFormat = GL_SRGB;
			else
				internalFormat = GL_RGB;

			format = GL_RGB;
		}

		else if (nrChannels == 4) {
			if (gamma_correction)
				internalFormat = GL_SRGB8_ALPHA8;
			else
				internalFormat = GL_RGBA;

			format = GL_RGBA;
		}

		// Load data into the current bound texture
		glBindTexture(GL_TEXTURE_2D, texture.m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texture.m_width, texture.m_height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}
	else
		throw std::string("Image not loaded: ") + str;

	// Free the image data
	stbi_image_free(data);

	return texture.m_id;
}
