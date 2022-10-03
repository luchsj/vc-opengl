/*#pragma once
#ifndef PRIMITIVE_
#define PRIMITIVE_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stb_image.h>
#include <string>
#include <vector>

//#include "shader.h"
//#include "mesh.h"
#include "model.h"

struct Material {
	glm::vec3 specular;
	float shininess;
};

struct Light {
	glm::vec3 position;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

class Primitive
{
public:
	Primitive(float* vertices_, Shader* shader_, std::string tex_path_ = "");
	~Primitive();
	void Draw();

protected:
	float* _vertices;
	std::string _texture_path;
	unsigned int _texture, VAO, VBO;

	Shader* _shader;

	glm::vec3 _position;
	glm::quat _rotation;
	glm::vec3 _scale;
	glm::vec3 _cameraPos;

	glm::mat4 _modelMatrix;
	glm::mat4 _view;
	glm::mat4 _projection;

	Material _material;
	Light _light;
};

Primitive::Primitive(float* vertices_, Shader* shader_, std::string tex_path_)
{
	_vertices = vertices_;
	_shader = shader_;

	_position = glm::vec3(0, 0, 0);
	_rotation = glm::quat();
	_scale = glm::vec3(0, 0, 0);

	glGenBuffers(1, &VBO);

	//create vertex array object to store our configuration
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//copy vertices into buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//copy vertex data into buffer memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

	//interpret vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//texture
	unsigned int _texture1;
	glGenTextures(1, &_texture1);
	glBindTexture(GL_TEXTURE_2D, _texture1);

	//set texture wrap parameters ((s,t) == (x, y))
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//set texture filtering to nearest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//load textures
	if (!tex_path_.empty())
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(tex_path_.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cerr << "Failed to load texture" << std::endl;
		}
		//free image memory
		stbi_image_free(data);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture1);
}

void Primitive::Draw()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	//cube 2
	_modelMatrix = glm::translate(glm::mat4(1.0f), _position);
	_modelMatrix = glm::scale(_modelMatrix, glm::vec3(5.0f, 5.0f, 0.5f));

	_shader->setMat4("model", _modelMatrix);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}
#endif;*/