#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in VS_OUT{
	vec2 texCoords;
	vec3 normal;
	vec3 fragPos;
	vec4 fragPosLightSpace;
}gs_in[];

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform float magnitude;
uniform float time;
uniform vec3 globalPos;

vec4 explode(vec4 position, vec3 normal)
{
	vec3 direction = vec3(0, 0, 0);
	//if(position.y < time - 1)
		direction = normal * magnitude;
		//direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
	return position + vec4(direction, 0.0);
}

vec3 getNormal()
{
	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(a, b));
}

void main()
{
	vec3 normal = getNormal();
	gl_Position = explode(gl_in[0].gl_Position, normal);
	TexCoords = gs_in[0].texCoords;
	Normal = gs_in[0].normal;
	FragPos = gs_in[0].fragPos;
	FragPosLightSpace = gs_in[0].fragPosLightSpace;

	EmitVertex();
	gl_Position = explode(gl_in[1].gl_Position, normal);
	TexCoords = gs_in[1].texCoords;
	Normal = gs_in[1].normal;
	FragPos = gs_in[1].fragPos;
	FragPosLightSpace = gs_in[1].fragPosLightSpace;

	EmitVertex();
	gl_Position = explode(gl_in[2].gl_Position, normal);
	TexCoords = gs_in[2].texCoords;
	Normal = gs_in[2].normal;
	FragPos = gs_in[2].fragPos;
	FragPosLightSpace = gs_in[2].fragPosLightSpace;

	//Normal = normal;

	EmitVertex();
	EndPrimitive();
}