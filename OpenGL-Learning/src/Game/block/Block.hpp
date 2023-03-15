#pragma once

#include <string>

#include "glm/glm.hpp"
#include "glm/gtx/normal.hpp"

#include "../primitive/Primitive.hpp"

namespace Minecraft {
	enum class BLOCKTYPE { STATIC_DEFAULT = 0, STATIC_TRANSPARENT = 1, DYNAMIC = 2, MESH = 3, AIR = 4, NONE = 5};

	struct Block_static {
		//glm::mat4 matrixModel{ 1.f };
		Minecraft::Vertex vertices[24];

		std::string name;
		unsigned int id;
		BLOCKTYPE subtype;

		float reflection = 0.f;

		glm::vec3 position;
		float size = 1.f;
	};

	struct Block_format {
		BLOCKTYPE type;
		std::string name;
		unsigned int id;

		float reflection = 0.f;

		std::string texture_top;
		std::string texture_bottom;
		std::string texture_left;
		std::string texture_right;
		std::string texture_back;
		std::string texture_front;
	};

	struct Texture_Format {
		std::string name;
		glm::vec2 uv[4];
	};
}