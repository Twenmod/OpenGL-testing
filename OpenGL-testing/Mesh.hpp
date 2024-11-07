struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture
{
	unsigned int id;
	std::string type;
	std::string path;
};


class Mesh
{
public:
	// mesh data
	std::vector<Vertex>       vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture>      textures;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(Shader& shader);
	void DrawInstanced(Shader& shader, unsigned int instances);
	void SetupInstanceData(unsigned int dataBuffer, unsigned int location, unsigned int size = 3, void* offset = (void*)0);
private:
	//  render data
	unsigned int VAO, VBO, EBO;

	void setupMesh();
};