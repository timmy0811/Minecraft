#pragma once

#include <glm/glm.hpp>

namespace OpenGL {
	struct Material {
		int texture;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shine;
	};
}
