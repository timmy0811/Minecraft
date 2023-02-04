#pragma once

#include "../debug/Debug.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/normal.hpp"

#include "../misc/Vertex.hpp"

class VertexBuffer {
private:
	unsigned int m_RendererID;
	size_t m_DataPtr;
	size_t m_BufferSize;

public:
	VertexBuffer(const void* data, unsigned int size);
	VertexBuffer(unsigned int count, size_t elementSize);
	~VertexBuffer();

	void AddVertexData(const void* data, int size, int offset);
	void AddVertexData(const void* data, int size);

	void Empty();

	static void addPoly(glm::vec3 pos0, glm::vec3 pos1, glm::vec3 pos2, glm::vec2 texPos0, glm::vec2 texPos1, glm::vec2 texPos2, glm::vec4 color, float texId, VertexBuffer& vb);
	static void addPolyN(glm::vec3 pos0, glm::vec3 pos1, glm::vec3 pos2, glm::vec2 texPos0, glm::vec2 texPos1, glm::vec2 texPos2, glm::vec4 color, float texId, VertexBuffer& vb);
	static void addPoly(glm::vec3 pos0, glm::vec3 pos1, glm::vec3 pos2, glm::vec4 color, VertexBuffer& vb);

	static void addCube(glm::vec3 position, float sideLength, glm::vec4 color, float texId, VertexBuffer& vb);
	static void addCubeN(glm::vec3 position, float sideLength, glm::vec4 color, float texId, VertexBuffer& vb);
	static void addLightSource(glm::vec3 position, float sideLength, glm::vec4 color, VertexBuffer& vb);

	void Bind() const;
	void Unbind() const;
};