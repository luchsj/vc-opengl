/*
#include "object.h"

//*** primitives ***

Object::Object()
{
	_position = glm::vec3(0, 0, 0);
	_rotation = glm::quat();
	_scale = glm::vec3(0, 0, 0);
	_modelMatrix = glm::mat4(1.0f);

	_shader = nullptr;
	_parent = nullptr;
	_mdl = nullptr;
}

Object::Object(std::string modelPath, Shader* shader_)
{
	_position = glm::vec3(0, 0, 0);
	_rotation = glm::quat();
	_scale = glm::vec3(0, 0, 0);
	_modelMatrix = glm::mat4(1.0f);

	_shader = shader_;
	_mdl = new Model(modelPath.c_str());
	_parent = nullptr;
}

Object::~Object()
{
	//for each (Object * child in children)
	//{
	//	delete(child);
	//}

	//delete(mdl);
}

void Object::translate(float x, float y, float z)
{
	_position = glm::vec3(x, y, z);
}

void Object::rotate(float theta, float x, float y, float z)
{
	_rotation = glm::rotate(_rotation, glm::radians(theta), glm::vec3(x, y, z));
}

void Object::scale(float x, float y, float z)
{
	_scale = glm::vec3(x, y, z);
}

void Object::setModel(std::string modelPath)
{
	if (!_mdl)
		_mdl = &Model(modelPath.c_str());
}

glm::mat4 Object::localMatrix()
{
	return glm::translate(glm::mat4(1.0f), _position) * glm::mat4_cast(_rotation) * glm::scale(glm::mat4(1.0f), _scale);
}

void Object::updateMatrices(glm::mat4 view, glm::mat4 projection)
{
	if (_parent)
		_modelMatrix = _parent->_modelMatrix * localMatrix();
	else
		_modelMatrix = localMatrix();

	for each (Object * child in _children)
	{
		child->updateMatrices(view, projection);
	}
}

void Object::setMaterial(glm::vec3 specular_, float shininess_)
{
	_material.specular = specular_;
	_material.shininess = shininess_;
}

void Object::setLight(glm::vec3 position_, glm::vec3 ambient_, glm::vec3 diffuse_, glm::vec3 specular_)
{
	_light.position = position_;
	_light.ambient = ambient_;
	_light.diffuse = diffuse_;
	_light.specular = specular_;
}

void Object::Draw()
{
	_mdl->Draw(*_shader);

	_shader->use();

	_modelMatrix = glm::translate(glm::mat4(1.0f), _position);
	_modelMatrix *= glm::mat4_cast(_rotation);
	_modelMatrix = glm::scale(_modelMatrix, _scale);

	glm::mat4 modelNormal = glm::transpose(glm::inverse(_modelMatrix));

	//***you gotta define these!***

	_projection = glm::perspective(glm::radians(30.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	_view = glm::lookAt(_cameraPos, _cameraPos + cameraFront, cameraUp);

	//vertex uniforms
	_shader->setMat4("view", _view);
	_shader->setMat4("projection", _projection);
	_shader->setMat4("model", _modelMatrix);
	_shader->setMat4("normalMatrix", modelNormal);

	//fragment uniforms
	_shader->setVec3("lightColor", .7f, 1.0f, .8f);
	_shader->setVec3("light.position", _light.position.x, _light.position.y, _light.position.z);
	_shader->setVec3("viewPos", _cameraPos);

	_shader->setVec3("material.specular", _material.specular.x, _material.specular.y, _material.specular.z);
	_shader->setFloat("material.shininess", _material.shininess);

	_shader->setVec3("light.ambient", _light.ambient.x, _light.ambient.y, _light.ambient.z);
	_shader->setVec3("light.diffuse", _light.diffuse.x, _light.diffuse.y, _light.diffuse.z);
	_shader->setVec3("light.specular", _light.specular.x, _light.specular.y, _light.specular.z);
}

Primitive::Primitive(float* vertices_, Shader* shader_, std::string tex_path_)
{
	_vertices = vertices_;

	_position = glm::vec3(0, 0, 0);
	_rotation = glm::quat();
	_scale = glm::vec3(0, 0, 0);

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	//create vertex array object to store our configuration
	unsigned int VAO;
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
	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	//set texture wrap parameters ((s,t) == (x, y))
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//set texture filtering to nearest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//load textures
	if(!tex_path_.empty())
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
	glBindTexture(GL_TEXTURE_2D, texture1);
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
*/