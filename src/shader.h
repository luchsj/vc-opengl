#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Shader
{
	public:
		unsigned int ID;

		Shader(const char* vertexPath, const char* fragmentPath)
		{
			//retrieve source code from fragment paths
			std::string vertexCode;
			std::string	fragmentCode;
			std::ifstream vShaderFile;
			std::ifstream fShaderFile;

			vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			
			try
			{
				vShaderFile.open(vertexPath);
				fShaderFile.open(fragmentPath);
				std::stringstream vShaderStream, fShaderStream;
				vShaderStream << vShaderFile.rdbuf();
				fShaderStream << fShaderFile.rdbuf();

				vShaderFile.close();
				fShaderFile.close();

				vertexCode = vShaderStream.str();
				fragmentCode = fShaderStream.str();
			}
			catch (std::ifstream::failure e)
			{
				std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
			}

			const char* vShaderCode = vertexCode.c_str();
			const char* fShaderCode = fragmentCode.c_str();

			//compile shaders
			unsigned int vertex, fragment;
			int success;
			char infoLog[512];

			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);
			glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vertex, 512, NULL, infoLog);
				std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << std::endl;
			}

			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, NULL);
			glCompileShader(fragment);
			glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED" << std::endl;
			}

			//shader program
			ID = glCreateProgram();
			glAttachShader(ID, vertex);
			glAttachShader(ID, fragment);
			glLinkProgram(ID);

			glGetProgramiv(ID, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(ID, 512, NULL, infoLog);
				std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			}

			glDeleteShader(vertex);
			glDeleteShader(fragment);
		}

		//overload for geometry shader
		Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath)
		{
			//retrieve source code from fragment paths
			std::string vertexCode;
			std::string geometryCode;
			std::string	fragmentCode;
			std::ifstream vShaderFile;
			std::ifstream gShaderFile;
			std::ifstream fShaderFile;

			vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

			try
			{
				vShaderFile.open(vertexPath);
				gShaderFile.open(geometryPath);
				fShaderFile.open(fragmentPath);
				std::stringstream vShaderStream, gShaderStream, fShaderStream;
				vShaderStream << vShaderFile.rdbuf();
				gShaderStream << gShaderFile.rdbuf();
				fShaderStream << fShaderFile.rdbuf();

				vShaderFile.close();
				gShaderFile.close();
				fShaderFile.close();

				vertexCode = vShaderStream.str();
				geometryCode = gShaderStream.str();
				fragmentCode = fShaderStream.str();
			}
			catch (std::ifstream::failure e)
			{
				std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
			}

			const char* vShaderCode = vertexCode.c_str();
			const char* gShaderCode = geometryCode.c_str();
			const char* fShaderCode = fragmentCode.c_str();

			//compile shaders
			unsigned int vertex, geometry, fragment;
			int success;
			char infoLog[512];

			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);
			glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vertex, 512, NULL, infoLog);
				std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << std::endl;
			}

			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
			glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(geometry, 512, NULL, infoLog);
				std::cerr << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED" << std::endl;
			}

			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, NULL);
			glCompileShader(fragment);
			glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED" << std::endl;
			}

			//shader program
			ID = glCreateProgram();
			glAttachShader(ID, vertex);
			glAttachShader(ID, geometry);
			glAttachShader(ID, fragment);
			glLinkProgram(ID);

			glGetProgramiv(ID, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(ID, 512, NULL, infoLog);
				std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			}

			glDeleteShader(vertex);
			glDeleteShader(geometry);
			glDeleteShader(fragment);
		}

		void use() //activate shader
		{
			glUseProgram(ID);
		}

		void setBool(const std::string& name, bool value) const
		{
			glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
		}
		void setInt(const std::string& name, int value) const
		{
			glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
		}
		void setFloat(const std::string& name, float value) const
		{
			glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
		}
		void setVec3(const std::string& name, float v1, float v2, float v3) const
		{
			glUniform3f(glGetUniformLocation(ID, name.c_str()), v1, v2, v3);
		}
		void setVec3(const std::string& name, glm::vec3 vec) const
		{
			glUniform3fv(glGetUniformLocation(ID, name.c_str()), 3, glm::value_ptr(vec));
		}
		void setMat4(const std::string& name, glm::mat4 mat) const
		{
			glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
		}
		void setMat3(const std::string& name, glm::mat3 mat) const
		{
			glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
		}

};
#endif