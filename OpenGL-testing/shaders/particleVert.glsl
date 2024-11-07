#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

struct Particle {
	vec3 position;
	vec3 velocity;
	float life;
	float lifeTime;
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};


void main()
{
    uint particleID = gl_InstanceID;
	vec3 particlePosition = particles[particleID].position;

    gl_Position = projection * view * vec4(aPos+particlePosition, 1.0);
	FragPos = aPos;
	Normal = aNormal;
	TexCoords = aTexCoords;
}