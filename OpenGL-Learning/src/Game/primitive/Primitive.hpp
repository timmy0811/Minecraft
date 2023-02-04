#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/normal.hpp"

namespace Minecraft {
	struct Vertex {
		glm::vec3 Position;
		glm::vec2 TexCoords;
		glm::vec3 Normal;
		float TexID;
	};
}