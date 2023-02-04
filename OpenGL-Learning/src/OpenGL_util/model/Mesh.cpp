#include "Mesh.h"

Mesh::Mesh(std::vector<OpenGL::VertexMesh> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures)
	:m_Vertices(vertices), m_Indices(indices), m_Textures(textures)
{
	// Load members
	m_IB = std::make_unique<IndexBuffer>(&m_Indices[0], (unsigned int)m_Indices.size());

	// Basic Setup
	m_VB = std::make_unique<VertexBuffer>(&m_Vertices[0], (unsigned int)(sizeof(OpenGL::VertexMesh) * m_Vertices.size()));
	m_VBLayout = std::make_unique<VertexBufferLayout>();

	m_VBLayout->Push<float>(3);	// Position
	m_VBLayout->Push<float>(2);	// TexCoord
	m_VBLayout->Push<float>(3); // Normal
	m_VBLayout->Push<float>(1);	// Texture Index

	m_VA = std::make_unique<VertexArray>();
	m_VA->AddBuffer(*m_VB, *m_VBLayout);
}

void Mesh::LoadTextures(Shader& shader)
{
	unsigned int diffuseInd = 0;
	unsigned int specularInd = 0;
	unsigned int shineIndex = 0;
	unsigned int normalIndex = 0;
	unsigned int heightInd = 0;

	unsigned int bindOffset = 1;

	// const size_t amountTex = m_Textures.size();

	constexpr unsigned int MAX_TEXTURE_SLOTS = 8;

	int samplerDiffuse[MAX_TEXTURE_SLOTS];
	int samplerSpecular[MAX_TEXTURE_SLOTS];
	int samplerShine[MAX_TEXTURE_SLOTS];
	int samplerNormal[MAX_TEXTURE_SLOTS];
	int samplerHeight[MAX_TEXTURE_SLOTS];

	// Clear Arrays to 0
	memset(samplerDiffuse, 0, sizeof(samplerDiffuse));
	memset(samplerSpecular, 0, sizeof(samplerSpecular));
	memset(samplerShine, 0, sizeof(samplerShine));
	memset(samplerNormal, 0, sizeof(samplerNormal));
	memset(samplerHeight, 0, sizeof(samplerHeight));
	
	shader.Bind();

	for (Texture* tex : m_Textures) {
		tex->Bind(bindOffset++);

		switch (tex->GetType()) {
		case TextureType::DIFFUSE:
			samplerDiffuse[diffuseInd++] = tex->GetBoundPort();
			break;
		case TextureType::SPECULAR:
			samplerSpecular[specularInd++] = tex->GetBoundPort();
			break;
		case TextureType::SHINE:
			samplerShine[shineIndex++] = tex->GetBoundPort();
			break;
		case TextureType::NORMAL:
			samplerNormal[normalIndex++] = tex->GetBoundPort();
			break;
		case TextureType::HEIGHT:
			samplerHeight[heightInd++] = tex->GetBoundPort();
			break;
		}
	}

	shader.SetUniform1iv("u_samplerDiffuse", MAX_TEXTURE_SLOTS, samplerDiffuse);
	shader.SetUniform1iv("u_samplerSpecular", MAX_TEXTURE_SLOTS, samplerSpecular);
	shader.SetUniform1iv("u_samplerShine", MAX_TEXTURE_SLOTS, samplerShine);
	shader.SetUniform1iv("u_samplerNormal", MAX_TEXTURE_SLOTS, samplerNormal);
	shader.SetUniform1iv("u_samplerHeight", MAX_TEXTURE_SLOTS, samplerHeight);
}

void Mesh::Draw(Shader& shader)
{
	LoadTextures(shader);

	m_Renderer.Draw(*m_VA, *m_IB, shader);
}
