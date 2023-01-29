#pragma once

#include <vector>

#include "../debug/Debug.hpp"

#include "../core/VertexBuffer.h"
#include "../core/VertexArray.h"
#include "../core/VertexBufferLayout.h"
#include "../core/IndexBuffer.h"
#include "../core/Renderer.h"

#include "../texture/Texture.h"

class Mesh
{
private:
	std::vector<VertexMesh> m_Vertices;
	std::vector<unsigned int> m_Indices;
	std::vector<Texture*> m_Textures;

	std::unique_ptr<VertexArray> m_VA;
	std::unique_ptr<VertexBuffer> m_VB;
	std::unique_ptr<VertexBufferLayout> m_VBLayout;
	std::unique_ptr<IndexBuffer> m_IB;

	Renderer m_Renderer;

	void LoadTextures(Shader& shader);

public:
	Mesh(std::vector<VertexMesh> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures);

	void Draw(Shader& shader);
};

