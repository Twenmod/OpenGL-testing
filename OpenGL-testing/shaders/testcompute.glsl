#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
	
struct Particle {
	vec3 position;
	vec3 velocity;
	float life;
	float lifeTime;
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

// variables
layout (location = 0) uniform float deltaTime;
layout (location = 1) uniform vec3 gravity = vec3(0.0, -9.8, 0.0);
layout (location = 2) uniform vec3 startPosition;

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

	if (particles[id].life <= 0) {
		particles[id].position = startPosition;
		return;
	}

	if (particles[id].life >= particles[id].lifeTime) {
		particles[id].position = startPosition;
		particles[id].life = 0;
		particles[id].velocity.y = 5;
	}
    particles[id].velocity += gravity * deltaTime;
    particles[id].position += particles[id].velocity * deltaTime;
}