#ifndef SCENE_
#define SCENE_

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

#include "shader.h"
#include "mesh.h"
#include "model.h"
#include "object.h"

class Scene
{
	public:
		Scene();

	protected:
		Object root;
};

#endif