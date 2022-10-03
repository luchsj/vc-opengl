#version 330 core
out vec4 FragColor;

struct Material{
	vec3 specular;
	float shininess;
};

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

float ShadowCalculation(vec4 fragPosLightSpace)
{
	vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
	projCoords = projCoords * .5 + .5;
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

	return shadow;
}

void main()
{
	vec3 color = texture(texture_diffuse1, TexCoords).rgb;
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(lightDir, norm), 0.0);
	vec3 diffuse = light.diffuse * diff;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), material.shininess);
	vec3 specular = light.specular * (spec * material.specular);

	vec3 ambient = 0.15 * light.diffuse;
	float shadow = ShadowCalculation(FragPosLightSpace);
	vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
	FragColor = vec4(result, 1.0);
}	