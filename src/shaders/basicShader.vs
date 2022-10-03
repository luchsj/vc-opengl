#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

//out vec3 fragPos;
//out vec3 normal;
//out vec2 TexCoords;

out VS_OUT{
	vec2 texCoords;
	vec3 normal;
	vec3 fragPos;
	vec4 fragPosLightSpace;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMatrix;
uniform mat4 lightSpaceMatrix;

void main()
{
	vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.normal = mat3(normalMatrix) * aNormal;
	vs_out.texCoords = aTexCoords;
	vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);

	//gl_Position = projection * view * vec4(fragPos, 1.0);
	gl_Position = projection * view * vec4(vec3(model * vec4(aPos, 1.0)), 1.0);
}