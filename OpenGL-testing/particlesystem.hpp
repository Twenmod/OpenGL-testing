#pragma once

class ComputeShader;
class Shader;
class Model;

struct Particle
{
	glm::vec3 position;
	float life = 0;
	glm::vec3 velocity;
	float lifeTime;
	glm::vec4 color;
	glm::vec4 endColor;
	float size;
	float endSize;
	float pad1;
	float pad2;
};

class ParticleSystem
{
public:
	ParticleSystem();
	ParticleSystem(float spawnInterval, float lifeTime = 2, glm::vec3 position = glm::vec3(0,0,0), float maxSpawnVelocity = 10);
	void Init(ComputeShader particleShader);
	void Tick(float deltaTime);
	void Draw(Model model, Shader shader);
private:
	ComputeShader m_particleShader;
	const float m_spawnInterval;
	float m_spawnTimer;
	const float m_particleLifeTime;
	const float m_spawnVelocity;
	const unsigned int m_particleAmount;
	glm::vec3 m_position;
	unsigned int m_SSBO;
	unsigned int m_atomicCounter;
};