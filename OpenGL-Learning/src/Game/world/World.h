#pragma once

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include "glm/gtc/matrix_transform.hpp"
#include "vendor/yaml/yaml_wrapper.hpp"
#include "config.h"

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
#include "../View/Camera.h"

// Util
#include "OpenGL_util/texture/Texture.h"
#include "OpenGL_util/core/Shader.h"
#include "OpenGL_util/misc/Light.hpp"
#include "Game/render/line.h"

class World
{
private:
	// Attributes ------------------------------------------
	glm::vec3 m_WorldRootPosition;
	glm::vec2 m_PlayerChunkPosition;

	glm::mat4 m_MatrixProjection;
	glm::mat4 m_MatrixView;
	glm::mat4 m_MatrixTranslation;

	// Objects
	std::vector<Chunk*> m_Chunks{ conf.WORLD_WIDTH * conf.WORLD_WIDTH };
	std::deque<Chunk*> m_ChunksQueuedGenerating, m_ChunksQueuedSerialize, m_ChunksQueuedDeserialize, m_ChunksQueuedCulling, m_ChunksQueuedBufferLoading;

	std::map<unsigned int, Minecraft::Block_format> m_BlockFormats;
	std::map<const std::string, Minecraft::Texture_Format> m_TextureFormats;
	std::set<std::string> m_UsedTextures;
	Texture m_TextureMap;

	Minecraft::Camera3D m_Camera;
	OpenGL::DirectionalLight m_DirLight;

	Minecraft::Render::ShaderPackage m_ShaderPackage;

	GLFWwindow* r_Window;

	Minecraft::Render::LineRenderer m_ChunkBorderRenderer;

	// Debug
	unsigned int m_DrawCalls = 0;

	// Noise
	siv::PerlinNoise m_Noise;

	// Threading
	std::mutex m_MutexLoading, m_MutexGenerating, m_MutexUnlaoding, m_MutexCullFaces, m_MutexBufferLoading;
	cyan::counting_semaphore<1000> m_GenerationSemaphore;

	std::atomic<int> m_IsGenerating;
	size_t m_GenerationThreadActions;
	bool m_ExecuteGenerationJob = true;
	bool m_IsGenerationInit = true;

	std::vector<std::thread> m_GenerationThreads;

	// Input
	float m_LastX = 400, m_LastY = 300;
	bool m_FirstMouseInit = true;

	// Methods ------------------------------------------
	// Setup
	void SetupChunkBorders();
	void SetupLight();
	void GenerateTerrain();

	void ParseBlocks(const std::string& path);
	void ParseTextures(const std::string& path);

	// Update
	void UpdateLight();

	void NeighborChunks();
	void HandleChunkLoading();

	static void OnMouseCallback(GLFWwindow* window, double xpos, double ypos);
	void ProcessMouse();

	// Misc
	inline int CoordToIndex(const glm::vec2& coord) const;
	inline const glm::vec2 IndexToCoord(unsigned int index) const;

	inline bool ContainsElementAtomic(std::deque<Chunk*>* list, std::mutex& mutex);
	template <typename T>
	bool ContainsElement(const std::deque<T>& queue, const T& element) const {
		return std::find(queue.begin(), queue.end(), element) != queue.end();
	}

public:
	// Attributes ------------------------------------------
	inline static float s_MouseX = 0;
	inline static float s_MouseY = 0;

	bool m_DrawChunkBorder;

	// Methods ------------------------------------------
	World(GLFWwindow* window);
	~World();

	// Setup
	void GenerationThreadJob();

	// Update
	void OnInput(GLFWwindow* window, double deltaTime);
	void OnRender();
	void OnUpdate(double deltaTime);

	// Misc

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

	inline const Minecraft::Render::ShaderPackage& getShaderPackage() { return m_ShaderPackage; };
};
