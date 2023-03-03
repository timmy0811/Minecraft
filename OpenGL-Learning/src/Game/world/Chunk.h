#pragma once

#include <vector>
#include <map>
#include <unordered_set>
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
	std::vector<Minecraft::Block_static*> m_BlockStatic{ conf.CHUNK_SIZE * conf.CHUNK_SIZE * conf.CHUNK_HEIGHT };
	std::vector<Minecraft::Block_static*> m_BlockTransparenStatic;

	std::vector<Minecraft::Vertex> m_VertexLoadBuffer;
	size_t m_LoadBufferPtr;
	bool m_WaitingForLoad;

	// block_static
	std::unique_ptr<VertexArray> m_VAstatic;
	std::unique_ptr<VertexBuffer> m_VBstatic;
	std::unique_ptr<IndexBuffer> m_IBstatic;
	std::unique_ptr<VertexBufferLayout> m_VBLayoutStatic;

	// block_transparent_static
	std::unique_ptr<VertexArray> m_VAtransparentStatic;
	std::unique_ptr<VertexBuffer> m_VBtransparentStatic;

	std::map<float, Minecraft::Face> m_TransparentStaticsOrdered;

	void OrderTransparentStatics(glm::vec3 cameraPosition);

	void FlushVertexBuffer(std::unique_ptr<VertexBuffer>& buffer);
	void LoadVertexBufferFromMap();
	void AddVertexBufferData(std::unique_ptr<VertexBuffer>& buffer, const void* data, size_t size);
	
	// Game
	unsigned int m_ID;
	size_t m_DrawnVertices;
	glm::vec3 m_Position;

	std::map<unsigned int, Minecraft::Block_format>* m_BlockFormats;
	std::map<const std::string, Minecraft::Texture_Format>* m_TextureFormats;

	// Generation Data
	glm::vec3 m_NoiseOffset;
	siv::PerlinNoise* m_Noise;

	// Chunk neighbors
	Chunk* m_ChunkNeighbors[4];

	Minecraft::Block_static CreateBlockStatic(const glm::vec3& position, unsigned int id);
	bool IsNotCovered(const glm::vec3 pos);
	inline unsigned int CoordToIndex(const glm::vec3& coord);
	inline const glm::vec3& IndexToCoord(unsigned int index);

	// Debug
	unsigned int m_DrawCalls = 0;
	bool m_IsGenerated = false;
	bool m_IsLoaded = false;

public:
	Chunk(std::map<unsigned int, Minecraft::Block_format>* blockFormatMap, std::map<const std::string, Minecraft::Texture_Format>* TextureFormatMap);
	~Chunk();

	void Generate();

	void OnRender(const Minecraft::Helper::ShaderPackage& shaderPackage, glm::vec3& cameraPosition);
	void OnRenderTransparents(const Minecraft::Helper::ShaderPackage& shaderPackage, glm::vec3& cameraPosition);
	void OnUpdate();

	// Buffers
	void Unload();
	void CullFacesOnLoadBuffer();
	void LoadVertexBufferFromLoadBuffer();

	void Serialize();
	void Deserialize();

	// Members
	void setGenerationData(const glm::vec3& position, const glm::vec3& noiseOffset, siv::PerlinNoise& noise);
	void setChunkNeighbors(Chunk* c1, Chunk* c2, Chunk* c3, Chunk* c4);
	void setChunkNeighbor(char index, Chunk* c);
	void setID(const unsigned int id) { m_ID = id; };

	inline bool IsGenerated() const { return m_IsGenerated; };
	inline bool isLoaded() const { return m_IsLoaded; };

	inline const bool getWaitingStatus() const { return m_WaitingForLoad; };
	inline const unsigned int getID() const { return m_ID; };
	Minecraft::Block_static** getBlocklistAllocator();
	inline const size_t getDrawnVertices() const { return m_DrawnVertices; }
	inline const size_t getAmountBlockStatic() const { return m_BlockStatic.size(); }

	inline const unsigned int getDrawCalls() {
		const unsigned int calls = m_DrawCalls;
		m_DrawCalls = 0;
		return calls;
	}
};
