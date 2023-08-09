#pragma once

#include <vector>
#include <map>
#include <unordered_set>
#include <time.h>

#include "PerlinNoise/Noise.hpp"

#include "config.h"

#include "OpenGL_util/core/Renderer.h"

#include "../render/CustomRenderer.h"
#include "../block/Block.hpp"
#include "../primitive/Primitive.hpp"

class Chunk
{
public:
	explicit Chunk(std::map<unsigned int, Minecraft::Block_format>* blockFormatMap, std::map<const std::string, Minecraft::Texture_Format>* TextureFormatMap);
	~Chunk();

	void OnUpdate();
	void OnRender(const Minecraft::Helper::ShaderPackage& shaderPackage);
	void OnRenderShadows(const Minecraft::Helper::ShaderPackage& shaderPackage);
	void OnRenderTransparents(const Minecraft::Helper::ShaderPackage& shaderPackage, const glm::vec3& cameraPosition);

	// Buffers
	unsigned int Generate();
	void CullFacesOnLoadBuffer();
	const bool SetBlock(const glm::vec3& position, unsigned int id, bool overwrite = false);
	const bool SetBlockUpdated(const glm::vec3& position, unsigned int id, bool overwrite = false);
	const int RemoveBlock(const glm::vec3& position);

	void LoadVertexBufferFromLoadBuffer();
	void Unload();

	void Serialize();
	void Deserialize();

	static unsigned long FastRandom512(void);

	// Accessors
	void setBiomeTemplate(std::vector<std::vector<Minecraft::Biome>>* biomes);
	void setStructureTemplate(std::vector<Minecraft::Structure>* structures);
	void setGenerationData(const glm::vec3& position, const glm::vec3& noiseOffsetH, const glm::vec3& noiseOffsetT, const glm::vec3& noiseOffsetM, Minecraft::GenerationNoise* noise);
	void setChunkNeighbors(Chunk* c1, Chunk* c2, Chunk* c3, Chunk* c4);
	void setChunkNeighbor(char index, Chunk* c);
	void setID(const unsigned int id) { m_ID = id; };
	inline void setSpawnFlag() { m_IsSpawnChunk = true; }

	inline const Minecraft::BLOCKTYPE getBlocktype(const glm::vec3& coord);
	inline const glm::vec3 getPosition() const { return m_Position; };
	inline const unsigned int getID() const { return m_ID; };
	inline const bool getWaitingStatus() const { return m_WaitingForLoad; }
	Minecraft::Block_static** getBlocklistAllocator();
	Minecraft::Block_static* getBlock(const glm::vec3& coord);
	inline const size_t getDrawnVertices() const { return m_DrawnVertices; }
	inline const size_t getAmountBlockStatic() const { return m_BlockStatic.size(); }
	inline const unsigned int getDrawCalls() {
		const unsigned int calls = m_DrawCalls;
		m_DrawCalls = 0;
		return calls;
	}

	inline bool IsGenerated() const { return m_IsGenerated; }
	inline bool isLoaded() const { return m_IsLoaded; }
	inline bool isSpawnChunk() const { return m_IsSpawnChunk; }

private:
	// Buffer
	void FlushVertexBuffer(std::unique_ptr<VertexBuffer>& buffer);
	void LoadVertexBufferFromMap();
	void AddVertexBufferData(std::unique_ptr<VertexBuffer>& buffer, const void* data, size_t size);

	void PushBlockToCollection(Minecraft::Block_static* block, const glm::vec3& position);

	// Culling
	void OrderTransparentStatics(glm::vec3 cameraPosition);
	Minecraft::Block_static CreateBlockStatic(const glm::vec3& position, unsigned int id);
	bool IsNotCovered(const glm::vec3 pos);

	// Misc
	inline unsigned int CoordToIndex(const glm::vec3& coord);
	inline const glm::vec3 IndexToCoord(unsigned int index);
	const glm::vec3 TranslateToWorldPosition(const glm::vec3& chunkPosition) const;

private:
	unsigned int m_ID;
	glm::vec3 m_Position;

	std::map<unsigned int, Minecraft::Block_format>* m_BlockFormats;
	std::map<const std::string, Minecraft::Texture_Format>* m_TextureFormats;

	// Generation Data
	glm::vec3 m_NoiseOffset_Height;
	glm::vec3 m_NoiseOffset_Temp;
	glm::vec3 m_NoiseOffset_Moist;
	Minecraft::GenerationNoise* m_Noise;
	bool m_IsSpawnChunk;

	std::vector<std::vector<Minecraft::Biome>>* m_BiomeTemplate;
	std::vector<Minecraft::Structure>* m_StructureTemplate;

	// Culling
	Chunk* m_ChunkNeighbors[4];
	std::vector<Minecraft::Vertex> m_VertexLoadBuffer;
	size_t m_LoadBufferPtr;
	bool m_WaitingForLoad;

	// block_static
	std::vector<Minecraft::Block_static*> m_BlockStatic{ conf.CHUNK_SIZE* conf.CHUNK_SIZE* conf.CHUNK_HEIGHT };

	std::unique_ptr<VertexArray> m_VAstatic;
	std::unique_ptr<VertexBuffer> m_VBstatic;
	std::unique_ptr<IndexBuffer> m_IBstatic;
	std::unique_ptr<VertexBufferLayout> m_VBLayoutStatic;

	// block_transparent_static
	std::vector<Minecraft::Block_static*> m_BlockTransparenStatic;

	std::unique_ptr<VertexArray> m_VAtransparentStatic;
	std::unique_ptr<VertexBuffer> m_VBtransparentStatic;

	std::map<float, Minecraft::Face> m_TransparentStaticsOrdered;

	// Debug
	size_t m_DrawnVertices;
	unsigned int m_DrawCalls = 0;

	// Flags
	bool m_IsGenerated = false;
	bool m_IsLoaded = false;
};
