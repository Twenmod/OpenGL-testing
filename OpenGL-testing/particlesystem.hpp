#pragma once

class ComputeShader;
class Shader;
class Model;

struct Particle
{
	glm::vec3 position;
	float pad0;
	glm::vec3 velocity;
	float life = 0;
	float lifeTime;
	float pad1;
	float pad2;
	float pad3;

};

class ParticleSystem
{
public:
	ParticleSystem();
	ParticleSystem(float spawnInterval, float lifeTime = 2, glm::vec3 position = glm::vec3(0,0,0));
	void Init(ComputeShader particleShader);
	void Tick(float deltaTime);
	void Draw(Model model, Shader shader);
private:
	ComputeShader m_particleShader;
	const float m_spawnInterval;
	const float m_particleLifeTime;
	const unsigned int m_particleAmount;
	glm::vec3 m_position;
	unsigned int m_SSBO;
};