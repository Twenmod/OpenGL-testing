#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;  
in vec2 TexCoords;
in vec3 FragPosViewSpace;
in vec4 FragPosLightSpace;


uniform vec3 viewPos;
uniform vec3 fogColor;
float FogFar = 500.0; 

struct Material {
    sampler2D diffuse;
    sampler2D specular;
	sampler2D emission;
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

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
#define NR_POINT_LIGHTS 0
uniform PointLight pointLights[NR_POINT_LIGHTS+1];

struct SpotLight {    
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
};  
uniform SpotLight spotLight;

uniform samplerCube cubemap;
float reflectAmount = 0.2f;

uniform sampler2D shadowMap;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);  
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);  
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	projCoords = projCoords * 0.5 + 0.5; 

	float closestDepth = texture(shadowMap, projCoords.xy).r;   
	float currentDepth = projCoords.z;  

	float bias = max(0.05 * (1.0 - dot(Normal, -dirLight.direction)), 0.005);
	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

	return shadow;
}


void main()
{

    vec4 texColor = texture(material.diffuse, TexCoords);

    if(texColor.a < 0.1)
        discard;

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

    // Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    result += vec4(CalcPointLight(pointLights[i], norm, FragPos, viewDir),0);    
    // Spot light
    result += vec4(CalcSpotLight(spotLight, norm, FragPos, viewDir),0);    
	
	//Skybox reflect
    vec3 reflec = reflect(-viewDir, normalize(Normal));
    result += vec4(texture(cubemap, reflec).rgb, 1) * reflectAmount;


	if (hasEmision) {
		vec3 emission = vec3(texColor);
		result += vec4(emission,0);
	}

	float alpha = texColor.a;
	result += vec4(0,0,0,alpha);

	float shadow = ShadowCalculation(FragPosLightSpace);       
	result = vec4(vec3(result)*(1-shadow),result.a);

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
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float dist    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + 
  			     light.quadratic * (dist * dist));    
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	if (light.diffuse == vec3(0)) {
		return vec3(0);
	}

    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}