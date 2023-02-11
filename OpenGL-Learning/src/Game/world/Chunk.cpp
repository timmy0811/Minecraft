#include "Chunk.h"

void Chunk::OrderTransparentStatics(glm::vec3 cameraPosition)
{
	m_TransparentStaticsOrdered.clear();
	unsigned int identifier = 0;
	for (auto iter = m_BlockTransparenStatic.begin(); iter != m_BlockTransparenStatic.end(); iter++) {
		Minecraft::Block_static block = *iter->second;
		for (int i = 0; i < 24; i += 4) {	// ???
			float distance = glm::length(cameraPosition - block.vertices[i].Position);
			distance += identifier++ * 0.000001f; // Add unique identifier

			Minecraft::Face face{};
			face.vertices[0] = block.vertices[i + 0];
			face.vertices[1] = block.vertices[i + 1];
			face.vertices[2] = block.vertices[i + 2];
			face.vertices[3] = block.vertices[i + 3];

			m_TransparentStaticsOrdered[distance] = face;
		}
	}
}

void Chunk::FlushVertexBuffer(std::unique_ptr<VertexBuffer>& buffer)
{
	buffer->Empty();
}

void Chunk::LoadVertexBuffer(std::unique_ptr<VertexBuffer>& buffer)
{
	// Prevent covered faces from getting drawn on the screen
	buffer->Empty();
	m_DrawnVertices = 0;
	auto iter = m_BlockStatic.begin();
	while (iter != m_BlockStatic.end()) {
		const glm::vec3 position = iter->second->position;

		// Front Face
		if (IsNotCovered({ position.x + 0, position.y + 0, position.z + c_BlockSize })) {
			buffer->AddVertexData(iter->second->vertices, sizeof(Minecraft::Vertex) * 4);
			m_DrawnVertices++;
		}

		// Right Face
		if (IsNotCovered({ position.x + c_BlockSize, position.y + 0, position.z + 0 })) {
			buffer->AddVertexData(iter->second->vertices + 4, sizeof(Minecraft::Vertex) * 4);
			m_DrawnVertices++;
		}

		// Left Face
		if (IsNotCovered({ position.x - c_BlockSize, position.y + 0, position.z + 0 })) {
			buffer->AddVertexData(iter->second->vertices + 8, sizeof(Minecraft::Vertex) * 4);
			m_DrawnVertices++;
		}

		// Back Face
		if (IsNotCovered({ position.x + 0, position.y + 0, position.z - c_BlockSize })) {
			buffer->AddVertexData(iter->second->vertices + 12, sizeof(Minecraft::Vertex) * 4);
			m_DrawnVertices++;
		}

		//  Top Face
		if (IsNotCovered({ position.x + 0, position.y + c_BlockSize, position.z + 0 })) {
			buffer->AddVertexData(iter->second->vertices + 16, sizeof(Minecraft::Vertex) * 4);
			m_DrawnVertices++;
		}

		// Bottom Face
		if (IsNotCovered({ position.x + 0, position.y - c_BlockSize, position.z + 0 })) {
			buffer->AddVertexData(iter->second->vertices + 20, sizeof(Minecraft::Vertex) * 4);
			m_DrawnVertices++;
		}

		iter++;
	}
}

void Chunk::LoadVertexBufferFromMap()
{
	for (std::map<float, Minecraft::Face>::reverse_iterator it = m_TransparentStaticsOrdered.rbegin(); it != m_TransparentStaticsOrdered.rend(); ++it) {
		AddVertexBufferData(m_VBtransparentStatic, it->second.vertices, sizeof(Minecraft::Vertex) * 4);
	}
}

void Chunk::AddVertexBufferData(std::unique_ptr<VertexBuffer>& buffer, const void* data, size_t size)
{
	buffer->AddVertexData(data, (int)size);
}

Minecraft::Block_static* Chunk::CreateBlockStatic(const glm::vec3& position, unsigned int id)
{
	Minecraft::Block_format& formatBlock = (*m_BlockFormats)[id];
	Minecraft::Block_static* block = new Minecraft::Block_static();

	block->name = (*m_BlockFormats)[id].name;
	block->id = id;
	block->position = position;
	block->reflection = formatBlock.reflection;
	block->subtype = formatBlock.type;

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

	glm::vec3 normals[6] = {
		{ 0, 0, 1 }, // F
		{ 1, 0, 0 }, // R
		{ -1, 0, 0 }, // L
		{ 0, 0, -1 }, // B
		{ 0, 1, 0 }, // U
		{ 0, -1, 0 }  // D
	};

	// Setting Texture IDs
	for (int i = 0; i < 24; i++) {
		block->vertices[i].TexID = 0.f;
	}

	// UVs
	// Front
	for (int i = 0; i < 4; i++) {
		block->vertices[i].TexCoords = (*m_TextureFormats)[(*m_BlockFormats)[id].texture_front].uv[i];
	}

	// Right
	for (int i = 0; i < 4; i++) {
		block->vertices[i + 4].TexCoords = (*m_TextureFormats)[(*m_BlockFormats)[id].texture_right].uv[i];
	}

	// Left
	for (int i = 0; i < 4; i++) {
		block->vertices[i + 8].TexCoords = (*m_TextureFormats)[(*m_BlockFormats)[id].texture_left].uv[i];
	}

	// Back
	for (int i = 0; i < 4; i++) {
		block->vertices[i + 12].TexCoords = (*m_TextureFormats)[(*m_BlockFormats)[id].texture_back].uv[i];
	}

	// Top
	for (int i = 0; i < 4; i++) {
		block->vertices[i + 16].TexCoords = (*m_TextureFormats)[(*m_BlockFormats)[id].texture_top].uv[i];
	}

	// Bottom
	for (int i = 0; i < 4; i++) {
		block->vertices[i + 20].TexCoords = (*m_TextureFormats)[(*m_BlockFormats)[id].texture_bottom].uv[i];
	}

	// Vertex position
	{
		block->vertices[0].Position = positions[0];
		block->vertices[1].Position = positions[1];
		block->vertices[2].Position = positions[5];
		block->vertices[3].Position = positions[4];

		block->vertices[4].Position = positions[1];
		block->vertices[5].Position = positions[2];
		block->vertices[6].Position = positions[6];
		block->vertices[7].Position = positions[5];

		block->vertices[8].Position = positions[3];
		block->vertices[9].Position = positions[0];
		block->vertices[10].Position = positions[4];
		block->vertices[11].Position = positions[7];

		block->vertices[12].Position = positions[2];
		block->vertices[13].Position = positions[3];
		block->vertices[14].Position = positions[7];
		block->vertices[15].Position = positions[6];

		block->vertices[16].Position = positions[4];
		block->vertices[17].Position = positions[5];
		block->vertices[18].Position = positions[6];
		block->vertices[19].Position = positions[7];

		block->vertices[20].Position = positions[3];
		block->vertices[21].Position = positions[2];
		block->vertices[22].Position = positions[1];
		block->vertices[23].Position = positions[0];
	}

	unsigned int face = 0;
	// UVs and Normals
	for (int i = 0; i < 24; i += 4) {
		block->vertices[i + 0].Normal = normals[face];
		block->vertices[i + 1].Normal = normals[face];
		block->vertices[i + 2].Normal = normals[face];
		block->vertices[i + 3].Normal = normals[face];

		block->vertices[i + 0].reflection = formatBlock.reflection;
		block->vertices[i + 1].reflection = formatBlock.reflection;
		block->vertices[i + 2].reflection = formatBlock.reflection;
		block->vertices[i + 3].reflection = formatBlock.reflection;

		face++;
	}

	return block;
}

bool Chunk::IsNotCovered(const glm::vec3& pos) const
{
	auto neighbor = m_BlockStatic.find({ pos.x, pos.y, pos.z });
	if (neighbor == m_BlockStatic.end() || neighbor->second->subtype != Minecraft::BLOCKTYPE::STATIC_DEFAULT) {
		return true;
	}
	else return false;
}

#pragma warning( push )
#pragma warning( disable : 4244 )

Chunk::Chunk(std::map<unsigned int, Minecraft::Block_format>* blockFormatMap, std::map<const std::string, Minecraft::Texture_Format>* textureFormatMap)
	:m_BlockFormats(blockFormatMap), m_TextureFormats(textureFormatMap)
{
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

	m_VBtransparentStatic = std::make_unique<VertexBuffer>(c_BatchFaceCount, sizeof(Minecraft::Vertex));

	delete[] indices;

	m_VBLayoutStatic = std::make_unique<VertexBufferLayout>();
	m_VBLayoutStatic->Push<float>(3);	// Position
	m_VBLayoutStatic->Push<float>(2);	// TexCoords
	m_VBLayoutStatic->Push<float>(3);   // Normal
	m_VBLayoutStatic->Push<float>(1);	// Texture Index
	m_VBLayoutStatic->Push<float>(1);	// Reflection

	m_VAstatic = std::make_unique<VertexArray>();
	m_VAtransparentStatic = std::make_unique<VertexArray>();

	m_VAstatic->AddBuffer(*m_VBstatic, *m_VBLayoutStatic);
	m_VAtransparentStatic->AddBuffer(*m_VBtransparentStatic, *m_VBLayoutStatic);

	// Setting seed for random integers
	std::srand((unsigned)time(NULL));
}

Chunk::~Chunk()
{
	// Freeing memory for opac objects
	for (auto iter = m_BlockStatic.begin(); iter != m_BlockStatic.end(); iter++) {
		delete iter->second;
	}
	m_BlockStatic.clear();

	// Freeing memory for transparent objects
	for (auto iter = m_BlockTransparenStatic.begin(); iter != m_BlockTransparenStatic.end(); iter++) {
		delete iter->second;
	}
	m_BlockTransparenStatic.clear();
}

#pragma warning( pop )

void Chunk::Generate(glm::vec3 position, glm::vec3 noiseOffset, siv::PerlinNoise& noise)
 {
	m_Position = position;

	constexpr double noiseStep = 1.f / c_ChunkSize;
	glm::vec2 noiseStepOffset = { 0.f, 0.f };

	// Generate Chunk using Perlin Noise offset
	for (int z = 0; z < c_ChunkSize; z++) {
		noiseStepOffset.x = 0.f;
		for (int x = 0; x < c_ChunkSize; x++) {
			double noiseOnTile = noise.octave2D_01(noiseOffset.x + noiseStepOffset.x, noiseOffset.y + noiseStepOffset.y, 1);
			unsigned int pillarHeight = (unsigned int)(noiseOnTile * c_TerrainYStretch);

			// Build Pillar depending on Noise
			for (unsigned int i = 0; i < pillarHeight; i++) {
				unsigned int id = (unsigned int)(std::floor(((float)rand() / RAND_MAX) * (m_BlockFormats->size() - 1)));	// Exclude last Block-ID -> Glass
				Minecraft::Block_static* block = CreateBlockStatic({ m_Position.x + x * c_BlockSize, m_Position.y + i * c_BlockSize, m_Position.z + z * c_BlockSize }, id);
				
				// Add block to specific buffer
				switch (block->subtype) {
				case Minecraft::BLOCKTYPE::STATIC_DEFAULT:
					m_BlockStatic[{block->position.x, block->position.y, block->position.z}] = block;
					//m_VBstatic->AddVertexData(block.vertices, (int)(sizeof(Minecraft::Vertex) * 24));		// Moved buffer loading into seperate function
					break;
				case Minecraft::BLOCKTYPE::STATIC_TRANSPARENT:
					m_BlockTransparenStatic[{block->position.x, block->position.y, block->position.z}] = block;
					// m_VBtransparentStatic->AddVertexData(block.vertices, (int)(sizeof(Minecraft::Vertex) * 24));
					break;
				}
			}
			noiseStepOffset.x += noiseStep;
		}
		noiseStepOffset.y += noiseStep;
	}
	LoadVertexBuffer(m_VBstatic);
}

void Chunk::OnRender(const Minecraft::Helper::ShaderPackage& shaderPackage, glm::vec3& cameraPosition)
{
	// Draw default static blocks
	m_DrawCalls++;
	shaderPackage.shaderBlockStatic->Bind();
	Renderer::Draw(*m_VAstatic, *m_IBstatic, *shaderPackage.shaderBlockStatic);
}

void Chunk::OnRenderTransparents(const Minecraft::Helper::ShaderPackage& shaderPackage, glm::vec3& cameraPosition)
{
	// Draw transparent static blocks
	OrderTransparentStatics(cameraPosition);
	FlushVertexBuffer(m_VBtransparentStatic);
	LoadVertexBufferFromMap();

	m_DrawCalls++;
	shaderPackage.shaderBlockStatic->Bind();
	Renderer::Draw(*m_VAtransparentStatic, *m_IBstatic, *shaderPackage.shaderBlockStatic);
}
