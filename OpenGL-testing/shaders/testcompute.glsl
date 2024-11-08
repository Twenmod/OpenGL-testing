#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
	
struct Particle {
	vec3 position;
	float life;
	vec3 velocity;
	float lifeTime;
	vec4 color;
	vec4 endColor;
	float size;
	float endSize;
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

// variables
layout (location = 0) uniform float deltaTime;
layout (location = 1) uniform vec3 gravity = vec3(0.0, -9.8, 0.0);
layout (location = 2) uniform vec3 startPosition;
layout (location = 3) uniform uint maxParticles;  
layout (location = 4) uniform uint particlesPerFrame;
layout (location = 5) uniform float spawnVelocity;

//Atomic counter to spawn particles in a controlled way
layout(binding = 1, offset = 0) uniform atomic_uint spawnIndex;

uint state;

uint xorshift()
{
	uint x = state;
	x ^= x << 13; // Shift left 13
	x ^= x >> 17; // Shift right 17
	x ^= x << 5;  // Shift left 5
	state = x;   // Update the state
	return x;
}

// Function to generate a random float between [0, 1]
float rand()
{
	return xorshift() * 2.3283064365387e-10;
}

uint randInt(uint range)
{
	return xorshift() % range;
}

uint pcg_hash(uint seed)
{
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}


void main() {
    uint id = gl_GlobalInvocationID.x;  // Particle index

	//For debugging
//	particles[id].position.x = 1;
//	particles[id].position.y = 2;
//	particles[id].position.z = 3;
//	particles[id].velocity.x = 4;
//	particles[id].velocity.y = 5;
//	particles[id].velocity.z = 6;
//	particles[id].life = 7;
//	particles[id].lifeTime = 8
	//return;

	particles[id].life += deltaTime;

	if (particles[id].life >= particles[id].lifeTime) {
		uint spawnIdx = atomicCounterIncrement(spawnIndex);
	    if (spawnIdx < particlesPerFrame) {
			state = pcg_hash(id);
			particles[id].position = startPosition;
			particles[id].life = 0;
			particles[id].velocity.x = rand()*spawnVelocity-spawnVelocity/2;
			particles[id].velocity.y = rand()*spawnVelocity-spawnVelocity/2;
			particles[id].velocity.z = rand()*spawnVelocity-spawnVelocity/2;

		}
	}
    particles[id].velocity += gravity * deltaTime;
    particles[id].position += particles[id].velocity * deltaTime;
}