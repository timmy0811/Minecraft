#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Minecraft {
	struct Camera3D {
		glm::vec3 Position{ 0.f, 0.f, 3.f };
		glm::vec3 Front{ 0.f, 0.f, -1.f };
		glm::vec3 Up{ 0.f, 1.f, 0.f };
		float yaw;
		float pitch;
	};
}