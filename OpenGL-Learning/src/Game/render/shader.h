#pragma once

#include "OpenGL_util/core/Shader.h"

namespace Minecraft::Render {
	struct ShaderPackage {
		Shader* shaderBlockStatic;

		~ShaderPackage() {
			delete shaderBlockStatic;
		}
	};
}