#pragma once

#include "OpenGL_util/core/Shader.h"

namespace Minecraft::Helper {
	// Structs
	struct Vec2_4 {
		glm::vec2 u0;
		glm::vec2 u1;
		glm::vec2 u2;
		glm::vec2 u3;
	};

	struct SymbolInformation {
		Vec2_4 uv;
		unsigned int width;
	};

	struct ShaderPackage {
		Shader* shaderBlockStatic;

		~ShaderPackage() {
			delete shaderBlockStatic;
		}
	};

	// Functions
	static int mapRGBToInt(const glm::vec3& color)
	{
		int r = int(color.r * 255) << 16;
		int g = int(color.g * 255) << 8;
		int b = int(color.b * 255);

		return r | g | b;
	}
}