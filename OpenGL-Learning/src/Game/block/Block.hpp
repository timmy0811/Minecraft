#pragma once

#include <string>

#include "glm/glm.hpp"
#include "glm/gtx/normal.hpp"

#include "../primitive/Primitive.hpp"

namespace Minecraft {
	struct Block_static {
		glm::mat4 matrixModel{ 1.f };

		Minecraft::Vertex vertices[24];
		std::string name;
		unsigned int id;

		glm::vec3 position;
		float size = 1.f;
	};
}