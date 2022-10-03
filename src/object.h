#ifndef OBJECT_
#define OBJECT_

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

class Object
{
	public:
		Object(std::string modelPath, Shader* shader_);
		~Object();

		void setModel(std::string modelPath);

		glm::vec3 getPosition() {return glm::vec3(_position);}
		glm::mat4 getRotation() {return glm::mat4(_rotation);}
		glm::vec3 getScale() {return glm::vec3(_scale);}
		glm::mat4 getMatrix() {return glm::mat4(_modelMatrix); }

		void translate(float x, float y, float z);
		
		void rotate(float theta, float x, float y, float z);
		void scale(float x, float y, float z);

		void setMaterial(glm::vec3 specular_, float shininess);
		void setLight(glm::vec3 position_, glm::vec3 ambient_, glm::vec3 diffuse_, glm::vec3 specular_);
		void setShader(Shader* s) {_shader = s;}
		void setCameraPos(glm::vec3 cameraPos_, glm::vec3 cameraFront){ _cameraPos = cameraPos_; _cameraFront = cameraFront;}
		void setScreenSize(float x, float y) {screenSizeX = x; screenSizeY = y;}
		void setFOV(float fov_) {fov = fov_;}

		void updateMatrices(glm::mat4 view, glm::mat4 projection);

		void setParent(Object parent_) {_parent = &parent_; _parent->_children.push_back(this);}

		glm::mat4 localMatrix();

		virtual void Draw();

	protected:
		Object(); //for interitors

		Shader* _shader;
		Model* _mdl;

		glm::vec3 _position;
		glm::quat _rotation;
		glm::vec3 _scale;
		glm::vec3 _cameraPos;
		glm::vec3 _cameraFront;

		glm::mat4 _modelMatrix;
		glm::mat4 _view;
		glm::mat4 _projection;

		std::vector<Object*> _children;
		Object* _parent;

		Material _material;
		Light _light;

		unsigned int screenSizeX, screenSizeY;
		float fov;
};

Object::Object()
{
	_position = glm::vec3(0, 0, 0);
	_rotation = glm::quat();
	_scale = glm::vec3(0, 0, 0);
	_modelMatrix = glm::mat4(1.0f);

	_shader = nullptr;
	_parent = nullptr;
	_mdl = nullptr;

	fov = 60.0f;
	screenSizeX = 600;
	screenSizeX = 800;
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

	fov = 60.0f;
	screenSizeX = 600;
	screenSizeX = 800;
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
	//check stuff
	if (!_shader)
	{
		std::cerr << "object.h got to draw with a null shader!" << std::endl;
		exit(1);
	}

	_shader->use();

	_modelMatrix = glm::translate(glm::mat4(1.0f), _position);
	_modelMatrix *= glm::mat4_cast(_rotation);
	_modelMatrix = glm::scale(_modelMatrix, _scale);

	glm::mat4 modelNormal = glm::transpose(glm::inverse(_modelMatrix));

	//***you gotta define these!***

	_projection = glm::perspective(glm::radians(fov), (float) screenSizeX / (float) screenSizeY, 0.1f, 100.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	_view = glm::lookAt(_cameraPos, _cameraPos + _cameraFront, cameraUp);

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

	_mdl->Draw(*_shader);
}
#endif