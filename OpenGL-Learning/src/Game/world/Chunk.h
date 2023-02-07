#pragma once

#include <vector>
#include <map>
#include <time.h>

#include "PerlinNoise/Noise.hpp"

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

	std::map<unsigned int, Minecraft::Block_format>* m_BlockFormats;
	std::map<const std::string, Minecraft::Texture_Format>* m_TextureFormats;

	Minecraft::Block_static CreateBlockStatic(const glm::vec3& position, unsigned int id);

	// Debug
	unsigned int m_DrawCalls = 0;

public:
	Chunk(std::map<unsigned int, Minecraft::Block_format>* blockFormatMap, std::map<const std::string, Minecraft::Texture_Format>* TextureFormatMap);

	void Generate(glm::vec3 position, glm::vec3 noiseOffset, siv::PerlinNoise& noise);

	void OnRender(const Minecraft::Helper::ShaderPackage& shaderPackage);
	void OnUpdate();

	// Members
	inline const size_t getAmountBlockStatic() const { return m_BlockStatic.size(); }
	inline const unsigned int getDrawCalls() {
		const unsigned int calls = m_DrawCalls;
		m_DrawCalls = 0;
		return calls;
	}
};
