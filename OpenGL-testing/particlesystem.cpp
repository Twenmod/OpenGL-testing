#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "computeshader.hpp"

#include "particlesystem.hpp"

#include "Shader.h"
#include "Model.hpp"
#include <vector>

ParticleSystem::ParticleSystem() :
	m_spawnInterval(0.1f),
	m_particleLifeTime(2.f),
	m_particleAmount(ceil(m_particleLifeTime / m_spawnInterval)),
	m_spawnVelocity(10)
{
	m_position = glm::vec3(0, 0, 0);
	m_SSBO = -1;
	m_spawnTimer = 0;
	m_atomicCounter = -1;
}

ParticleSystem::ParticleSystem(float spawnInterval, float lifeTime, glm::vec3 position, float maxSpawnVelocity) :
	m_spawnInterval(spawnInterval),
	m_particleLifeTime(lifeTime),
	m_particleAmount(std::max(round(m_particleLifeTime / m_spawnInterval),1.f)),
	m_spawnVelocity(maxSpawnVelocity)
{
	m_position = position;
	m_SSBO = -1;
	m_spawnTimer = 0;
	m_atomicCounter = -1;
}

void ParticleSystem::Init(ComputeShader particleShader)
{
	m_particleShader = particleShader;

	//Create particle array
	Particle* particles = new Particle[m_particleAmount];
	const float spawnRate = 1;
	for (int i = 0; i < (int)m_particleAmount; i++)
	{
		particles[i].position = m_position;
		particles[i].velocity = glm::vec3(rand() / (float)RAND_MAX * 10 - 5, 0, rand() / (float)RAND_MAX * 10 - 5);
		particles[i].color = glm::vec4(1, 0, 0, 1);
		particles[i].endColor = glm::vec4(1, 0, 0, 1);
		particles[i].lifeTime = m_particleLifeTime;
		particles[i].life = m_particleLifeTime;
		particles[i].size = 1;
		particles[i].endSize = 0;
	}

	//Create buffer
	glGenBuffers(1, &m_SSBO);
	//Bind data
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * m_particleAmount, particles, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_SSBO);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	//Unbind and free
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	//Create atomic counter
	glGenBuffers(1, &m_atomicCounter);
	// Bind and allocate memory for the atomic counter
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounter);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
	// Reset the counter to 0
	GLuint zero = 0;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);

	// Bind the buffer to the binding point 1 (as in your shader)
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, m_atomicCounter);

	delete[] particles;
}

void ParticleSystem::Tick(float deltaTime)
{
	//Setup data
	m_particleShader.use();
	m_particleShader.setFloat("deltaTime", deltaTime);
	m_particleShader.setVec3("startPosition", m_position);
	m_particleShader.setUInt("maxParticles", m_particleAmount);
	m_particleShader.setUInt("particlesPerFrame", ceil(deltaTime/m_spawnInterval));
	m_particleShader.setFloat("spawnVelocity", m_spawnVelocity);

	m_spawnTimer += deltaTime;
	if (m_spawnTimer > m_spawnInterval)
	{
		m_spawnTimer = 0;
		//Reset the atomic counter
		GLuint zero = 0;
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounter);
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
	}
	//Bind buffer
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_SSBO);
	//Compute
	glDispatchCompute((unsigned int)m_particleAmount, 1, 1);
	// make sure writing finished
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


	//Debug for allignement fixing
	// 
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);
	// Map the buffer to the CPU address space
	//Particle* mappedBuffer = (Particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//if (mappedBuffer)
	//{
	//	for (int i = 0; i < particleAmount; i++)
	//	{
	//		std::cout << "Particle " << i << ": Pos: ("
	//			<< mappedBuffer[i].position.x << ", "
	//			<< mappedBuffer[i].position.y << ", "
	//			<< mappedBuffer[i].position.z << "), Vel: ("
	//			<< mappedBuffer[i].velocity.x << ", "
	//			<< mappedBuffer[i].velocity.y << ", "
	//			<< mappedBuffer[i].velocity.z << "), Life: "
	//			<< mappedBuffer[i].life << ", time: "
	//			<< mappedBuffer[i].lifeTime << std::endl;
	//	}
	//}
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ParticleSystem::Draw(Model model, Shader shader)
{
	shader.use();

	//Bind buffers
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_SSBO);

	model.DrawInstanced(shader , m_particleAmount);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}