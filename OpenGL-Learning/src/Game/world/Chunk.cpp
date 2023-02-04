#include "Chunk.h"

void Chunk::FlushVertexBuffer()
{
	m_VBstatic->Empty();
}

void Chunk::LoadVertexBuffer()
{
	m_VBstatic->Empty();
	for (Minecraft::Block_static& block : m_BlockStatic) {
		m_VBstatic->AddVertexData(block.vertices, sizeof(Minecraft::Vertex) * 24);
	}
}

void Chunk::AddVertexBufferData(const void* data, size_t size)
{
	m_VBstatic->AddVertexData(data, (int)size);
}

Minecraft::Block_static Chunk::CreateBlockStatic(const glm::vec3& position, unsigned int id)
{
	Minecraft::Block_static block;
	block.name = "";
	block.id = id;
	block.position = position;

	glm::vec3 positions[8] = {
		{position.x + 0, position.y + 0, position.z - 0},	// 0 FDL
		{position.x + 1, position.y + 0, position.z - 0},	// 1 FDR
		{position.x + 1, position.y + 0, position.z - 1},	// 2 BDR
		{position.x + 0, position.y + 0, position.z - 1},	// 3 BDL

		{position.x + 0, position.y + 1, position.z - 0},	// 4 FUL
		{position.x + 1, position.y + 1, position.z - 0},	// 5 FUR
		{position.x + 1, position.y + 1, position.z - 1},	// 6 BUR
		{position.x + 0, position.y + 1, position.z - 1}	// 7 BUL
	};

	glm::vec2 uvs[4] = {
		{0.f, 0.f},
		{1.f, 0.f},
		{1.f, 1.f},
		{0.f, 1.f}
	};

	glm::vec3 normals[6] = {
		{ 0, 0, 1 }, // F
		{ 1, 0, 0 }, // R
		{ -1, 0, 0 }, // L
		{ 0, 0, -1 }, // B
		{ 0, 1, 0 }, // U
		{ 0, -1, 0 }  // D
	};

	// TODO: Determine Texture ID

	// Setting Texture IDs
	// Front
	for (int i = 0; i < 4; i++) {
		block.vertices[i].TexID = (float)0;
	}

	// Right
	for (int i = 4; i < 8; i++) {
		block.vertices[i].TexID = (float)0;
	}

	// Left
	for (int i = 8; i < 12; i++) {
		block.vertices[i].TexID = (float)0;
	}

	// Back
	for (int i = 12; i < 16; i++) {
		block.vertices[i].TexID = (float)0;
	}

	// Top
	for (int i = 16; i < 20; i++) {
		block.vertices[i].TexID = (float)1;
	}

	// Bottom
	for (int i = 20; i < 24; i++) {
		block.vertices[i].TexID = (float)1;
	}

	// Vertex position
	{
		block.vertices[0].Position = positions[0];
		block.vertices[1].Position = positions[1];
		block.vertices[2].Position = positions[5];
		block.vertices[3].Position = positions[4];

		block.vertices[4].Position = positions[1];
		block.vertices[5].Position = positions[2];
		block.vertices[6].Position = positions[6];
		block.vertices[7].Position = positions[5];

		block.vertices[8].Position = positions[3];
		block.vertices[9].Position = positions[0];
		block.vertices[10].Position = positions[4];
		block.vertices[11].Position = positions[7];

		block.vertices[12].Position = positions[2];
		block.vertices[13].Position = positions[3];
		block.vertices[14].Position = positions[7];
		block.vertices[15].Position = positions[6];

		block.vertices[16].Position = positions[4];
		block.vertices[17].Position = positions[5];
		block.vertices[18].Position = positions[6];
		block.vertices[19].Position = positions[7];

		block.vertices[20].Position = positions[3];
		block.vertices[21].Position = positions[2];
		block.vertices[22].Position = positions[1];
		block.vertices[23].Position = positions[0];
	}

	unsigned int face = 0;
	// UVs and Normals
	for (int i = 0; i < 24; i += 4) {
		block.vertices[i + 0].TexCoords = uvs[0];
		block.vertices[i + 1].TexCoords = uvs[1];
		block.vertices[i + 2].TexCoords = uvs[2];
		block.vertices[i + 3].TexCoords = uvs[3];

		block.vertices[i + 0].Normal = normals[face];
		block.vertices[i + 1].Normal = normals[face];
		block.vertices[i + 2].Normal = normals[face];
		block.vertices[i + 3].Normal = normals[face];

		face++;
	}

	return block;
}

#pragma warning( push )
#pragma warning( disable : 4244 )

Chunk::Chunk()
{
	m_Position = { 0.f, 0.f, 0.f };

	// Heap allocate Indix-Buffer
	unsigned int* indices = new unsigned int[c_BatchFaceCount * 6];
	unsigned int offset = 0;

	for (size_t i = 0; i < c_BatchFaceCount * 6; i += 6) {
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;

		indices[i + 3] = 2 + offset;
		indices[i + 4] = 3 + offset;
		indices[i + 5] = 0 + offset;

		offset += 4;
	}

	m_IBstatic = std::make_unique<IndexBuffer>(indices, c_BatchFaceCount * 6);
	m_VBstatic = std::make_unique<VertexBuffer>(c_BatchFaceCount, sizeof(Minecraft::Vertex));

	delete[] indices;

	m_VBLayoutStatic = std::make_unique<VertexBufferLayout>();
	m_VBLayoutStatic->Push<float>(3);	// Position
	m_VBLayoutStatic->Push<float>(2);	// TexCoords
	m_VBLayoutStatic->Push<float>(3);   // Normal
	m_VBLayoutStatic->Push<float>(1);	// Texture Index

	m_VAstatic = std::make_unique<VertexArray>();
	m_VAstatic->AddBuffer(*m_VBstatic, *m_VBLayoutStatic);
}

#pragma warning( pop )

void Chunk::Generate()
{
	// Example Setup 
	for (int z = 0; z < c_ChunkSize; z++) {
		for (int x = 0; x < c_ChunkSize; x++) {
			m_BlockStatic.push_back(CreateBlockStatic({ m_Position.x + x * c_BlockSize, m_Position.y + 0 * c_BlockSize, m_Position.z + z * c_BlockSize }, 0));
			AddVertexBufferData(m_BlockStatic[m_BlockStatic.size() - 1].vertices, sizeof(Minecraft::Vertex) * 24);
		}
	}
}

void Chunk::OnRender(const Minecraft::Helper::ShaderPackage& shaderPackage)
{
	shaderPackage.shaderBlockStatic->Bind();
	Renderer::Draw(*m_VAstatic, *m_IBstatic, *shaderPackage.shaderBlockStatic);
}
