#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 Color;
out vec3 FragPosViewSpace;

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


void main()
{

    uint particleID = gl_InstanceID;
	Particle particle = particles[particleID];

	if (particle.life > particle.lifeTime) {
		Color = vec4(0,0,0,0);
		gl_Position = vec4(0,0,0,0);
		return;
	}

	float scalar = mix(particle.size,particle.endSize,particle.life/particle.lifeTime);
	mat4 scalematrix = mat4(scalar, 0, 0, 0,
							0,scalar,  0, 0,
							0,0,scalar,   0,
							0,0, 0,       1);

	mat4 translation = mat4(1, 0, 0, 0,
							0, 1, 0, 0,
							0, 0, 1, 0,
							particle.position.x, particle.position.y, particle.position.z, 1);
    gl_Position = projection * view * translation * scalematrix * vec4(aPos, 1.0);
	FragPos = vec3(translation * vec4(aPos,1));
	FragPosViewSpace = vec3(view * translation * vec4(aPos,1.0f));
	Normal = aNormal;
	TexCoords = aTexCoords;
	Color = mix(particle.color,particle.endColor,particle.life/particle.lifeTime);
}