#pragma once

#include <vector>

#include "config.h"

#include "OpenGL_util/core/VertexBuffer.h"
#include "OpenGL_util/core/VertexArray.h"
#include "OpenGL_util/core/IndexBuffer.h"
#include "OpenGL_util/core/VertexBufferLayout.h"
#include "OpenGL_util/core/Shader.h"
#include "OpenGL_util/core/Renderer.h"

#include "../block/Block.hpp"
#include "../primitive/Primitive.hpp"

namespace Minecraft {
	namespace Helper {
		struct ShaderPackage {
			Shader* shaderBlockStatic;

			~ShaderPackage() {
				delete shaderBlockStatic;
			}
		};
	}
}

class Chunk
{
private:
	// Graphics
	std::vector<Minecraft::Block_static> m_BlockStatic;

	std::unique_ptr<VertexArray> m_VAstatic;
	std::unique_ptr<VertexBuffer> m_VBstatic;
	std::unique_ptr<IndexBuffer> m_IBstatic;
	std::unique_ptr<VertexBufferLayout> m_VBLayoutStatic;

	void FlushVertexBuffer();
	void LoadVertexBuffer();
	void AddVertexBufferData(const void* data, size_t size);

	// Game
	glm::vec3 m_Position;

	Minecraft::Block_static CreateBlockStatic(const glm::vec3& position, unsigned int id);

public:
	Chunk();

	void Generate();

	void OnRender(const Minecraft::Helper::ShaderPackage& shaderPackage);
	void OnUpdate();
};
