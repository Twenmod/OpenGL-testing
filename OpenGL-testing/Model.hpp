#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Texture.hpp"
#include "Mesh.hpp"

enum class Primitives
{
	PRIMITIVE_PLANE,
	PRIMITIVE_CUBE
};

class Model
{
public:
	Model(const char* path)
	{
		loadModel(path);
	}
	Model(Primitives model, TextureObject diffuse, TextureObject specular = TextureObject(), TextureObject emission = TextureObject())
	{
		loadModel(model, diffuse, specular, emission);
	}
	Model(Primitives model)
	{
		loadModel(model);
	}
	Model() {}
	void LoadCubeMap(TextureObject cubemap);
	void Draw(Shader& shader);
	void DrawInstanced(Shader& shader, unsigned int instances);
	void SetupInstanceData(unsigned int dataBuffer, unsigned int location, unsigned int size = 3, void* offset = (void*)0);
private:
	// model data
	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<Texture> textures_loaded;

	void loadModel(std::string path);
	void loadModel(Primitives model, TextureObject diffuse = TextureObject(), TextureObject specular = TextureObject(), TextureObject emission = TextureObject());
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
		std::string typeName);
	unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = true);
};