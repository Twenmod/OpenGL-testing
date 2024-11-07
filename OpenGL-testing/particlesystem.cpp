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
	m_particleAmount(ceil(m_particleLifeTime / m_spawnInterval))
{
	m_position = glm::vec3(0, 0, 0);
	m_SSBO = -1;
}

ParticleSystem::ParticleSystem(float spawnInterval, float lifeTime, glm::vec3 position) :
	m_spawnInterval(spawnInterval),
	m_particleLifeTime(lifeTime),
	m_particleAmount(ceil(m_particleLifeTime / m_spawnInterval))
{
	m_position = position;
	m_SSBO = -1;
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
		particles[i].life = -(i+1) * m_spawnInterval;
		particles[i].lifeTime = m_particleLifeTime;
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
	delete[] particles;
}

void ParticleSystem::Tick(float deltaTime)
{
	//Setup data
	m_particleShader.use();
	m_particleShader.setFloat("deltaTime", deltaTime);
	m_particleShader.setVec3("startPosition", m_position);
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

	glm::mat4 tmodel = glm::mat4(1.0f);
	m_particleShader.setMat4("model", tmodel);
	model.DrawInstanced(shader , m_particleAmount);
}