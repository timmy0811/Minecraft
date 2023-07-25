#pragma once

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include "glm/gtc/matrix_transform.hpp"
#include "vendor/yaml/yaml_wrapper.hpp"
#include "config.h"

// Game
#include "Game/render/CustomRenderer.h"
#include "Game/view/CharacterController.h"

// System
#include "windowsWrapper.h"
#include <functional>
#include <time.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

// Data structures
#include <vector>
#include <map>
#include <set>
#include <queue>

#include <mutex>
#include "threading/semaphore.h"

#include "Chunk.h"

// Util
#include "OpenGL_util/texture/Texture.h"
#include "OpenGL_util/core/Shader.h"
#include "OpenGL_util/misc/Light.hpp"

class World
{
public:
	World(GLFWwindow* window);
	~World();

	// Setup
	void GenerationThreadJob();

	// Update
	void OnInput(GLFWwindow* window, double deltaTime);
	void OnRender();
	void OnUpdate(double deltaTime);
	void OnWindowResize();

	// Misc
	void UpdateProjectionMatrix(float FOV, float nearD = 0.1f, float farD = 300.f);

	// Accessors
	const glm::mat4& getMatrixProjection() const;
	const glm::mat4& getMatrixView() const;
	const glm::vec3& getCameraPosition() const;

	const unsigned int getAmountBlockStatic() const;

	inline const size_t getAmountChunk() const { return m_Chunks.size(); }
	inline const size_t getAmountBlockFormat() const { return m_BlockFormats.size(); };
	inline const size_t getAmountTextureFormat() const { return m_TextureFormats.size(); }
	const size_t getDrawnVertices() const;
	const unsigned int getDrawCalls() const;

	inline const Minecraft::Helper::ShaderPackage& getShaderPackage() { return m_ShaderPackage; };

	inline void toggleCharacterMode(Minecraft::CharacterController::STATE mode) { m_CharacterController.toggleMode(mode); };

public:
	bool m_DrawChunkBorder;

private:
	// Setup
	void SetupChunkBorders();
	void SetupLight();
	void GenerateTerrain();

	void ParseBlocks(const std::string& path);
	void ParseTextures(const std::string& path);
	void ParseBiomes(const std::string& path);
	void ParseStructures(const std::string& path);

	// Update
	void UpdateLight();

	void NeighborChunks();
	void HandleChunkLoading();

	void OutlineSelectedBlock();

	// Misc
	inline Chunk* CoordToChunkSecure(const glm::vec2& coord);
	inline int CoordToChunkIndex(const glm::vec2& coord) const;
	inline const glm::vec2 IndexToCoord(unsigned int index) const;

	inline bool ContainsElementAtomic(std::deque<Chunk*>* list, std::mutex& mutex);
	template <typename T>
	bool ContainsElement(const std::deque<T>& queue, const T& element) const {
		return std::find(queue.begin(), queue.end(), element) != queue.end();
	}

private:
	unsigned int m_CountChunks;
	glm::vec3 m_WorldRootPosition;
	glm::vec2 m_PlayerChunkPosition;

	glm::mat4 m_MatrixProjection;
	glm::mat4 m_MatrixView;
	glm::mat4 m_MatrixTranslation;

	// Objects
	CharacterController m_CharacterController;

	std::vector<Chunk*> m_Chunks{ conf.WORLD_WIDTH* conf.WORLD_WIDTH };
	std::deque<Chunk*> m_ChunksQueuedGenerating, m_ChunksQueuedSerialize, m_ChunksQueuedDeserialize, m_ChunksQueuedCulling, m_ChunksQueuedBufferLoading;

	std::map<unsigned int, Minecraft::Block_format> m_BlockFormats;
	std::map<const std::string, Minecraft::Texture_Format> m_TextureFormats;
	std::set<std::string> m_UsedTextures;
	Texture m_TextureMap;

	OpenGL::DirectionalLight m_DirLight;

	Minecraft::Helper::ShaderPackage m_ShaderPackage;

	GLFWwindow* r_Window;

	Minecraft::Helper::LineRenderer m_ChunkBorderRenderer;
	Minecraft::Helper::BlockSelectionRenderer m_BlockSelectionRenderer;
	Minecraft::Helper::SpriteRenderer m_HUDRenderer;

	std::vector<std::vector<Minecraft::Biome>> m_BiomeTemplate;
	std::vector<Minecraft::Structure> m_StructureTemplate;

	// Debug
	unsigned int m_DrawCalls = 0;

	// Noise
	Minecraft::GenerationNoise m_GenerationNoise;

	// Threading
	std::mutex m_MutexLoading, m_MutexGenerating, m_MutexUnlaoding, m_MutexCullFaces, m_MutexBufferLoading;
	cyan::counting_semaphore<1000> m_GenerationSemaphore;

	std::atomic<int> m_IsGenerating;
	size_t m_GenerationThreadActions;
	bool m_ExecuteGenerationJob = true;
	bool m_IsGenerationInit = true;

	std::vector<std::thread> m_GenerationThreads;
};
