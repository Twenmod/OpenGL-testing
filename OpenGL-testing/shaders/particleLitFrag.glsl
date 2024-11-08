#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;  
in vec2 TexCoords;
in vec4 Color;
in vec3 FragPosViewSpace;

struct Material {
    float shininess;
};
uniform Material material;
uniform bool hasEmision;

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

uniform samplerCube cubemap;
float reflectAmount = 0.5f;

uniform vec3 viewPos;
uniform vec3 fogColor;
float FogFar = 50000.0; 

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);  

void main()
{
	if (Color.a == 0) discard;

	float depth = length(FragPosViewSpace);

	float fogMix = clamp((depth*depth)/FogFar,0,1);

	if (fogMix >= 1) {
		discard;
	}else {
	    // properties
		vec3 norm = normalize(Normal);
		vec3 viewDir = normalize(viewPos - FragPos);

		// Directional lighting
		vec4 result = vec4(CalcDirLight(dirLight, norm, viewDir),0);

		//Skybox reflect
		vec3 reflec = reflect(-viewDir, normalize(Normal));
		result += vec4(texture(cubemap, reflec).rgb, 1) * reflectAmount;

		float alpha = Color.a;
		result += vec4(0,0,0,alpha);
		
		result = vec4(mix(vec3(result), fogColor, fogMix),result.a);
		FragColor = result;

	}
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient  = light.ambient  * vec3(Color);
    vec3 diffuse  = light.diffuse  * diff * vec3(Color);
    return (ambient + diffuse);
}