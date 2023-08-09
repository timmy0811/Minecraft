#include "SpriteRenderer.h"

Minecraft::Helper::SpriteRenderer::SpriteRenderer(int maxSprites, glm::uvec2 samplerSlotRange, const std::string& shaderVert, const std::string& shaderFrag)
	: m_Shader(new Shader(shaderVert, shaderFrag)), m_SamplerRangeLow(samplerSlotRange.x), m_SamplerRangeHigh(samplerSlotRange.y) {
	m_MatProjection = glm::ortho(0.0f, (float)conf.WIN_WIDTH, 0.0f, (float)conf.WIN_HEIGHT, -1.0f, 1.0f);
	m_MatTranslation = glm::vec3(0.f, 0.f, 0.f);
	m_MatView = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f));

	unsigned int* indices = new unsigned int[maxSprites * 6];

	unsigned int offset = 0;
	for (size_t i = 0; i < maxSprites * 6; i += 6) {
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;

		indices[i + 3] = 2 + offset;
		indices[i + 4] = 3 + offset;
		indices[i + 5] = 0 + offset;

		offset += 4;
	}

	m_IB = std::make_unique<IndexBuffer>(indices, maxSprites * 6);
	m_VB = std::make_unique<VertexBuffer>(maxSprites * 4, sizeof(Minecraft::Sprite2DVertex));

	delete[] indices;

	m_VBLayout = std::make_unique<VertexBufferLayout>();
	m_VBLayout->Push<float>(2);	// Position
	m_VBLayout->Push<float>(2);	// UVs
	m_VBLayout->Push<float>(1);	// Texture Index

	m_VA = std::make_unique<VertexArray>();
	m_VA->AddBuffer(*m_VB, *m_VBLayout);

	m_Shader->Bind();
	m_Shader->SetUniformMat4f("u_MVP", m_MatProjection * m_MatView * glm::translate(glm::mat4(1.f), m_MatTranslation));
}

Minecraft::Helper::SpriteRenderer::~SpriteRenderer()
{
	for (auto it = m_Sprites.begin(); it != m_Sprites.end(); it++)
	{
		delete it->second;
	}
}

void Minecraft::Helper::SpriteRenderer::Draw() const
{
	GLContext::Draw(*m_VA, *m_IB, *m_Shader, GL_TRIANGLES, m_SpritesOnScreen.size() * 6);
	m_Shader->Unbind();
}

void Minecraft::Helper::SpriteRenderer::PushAll()
{
	m_VB->Bind();
	m_VB->Empty();
	m_Triangles = m_Sprites.size() * 2;

	for (auto it = m_Sprites.begin(); it != m_Sprites.end(); it++)
	{
		m_VB->AddVertexData(it->second->vertices, sizeof(Minecraft::Sprite2DVertex) * 4);
	}
}

void Minecraft::Helper::SpriteRenderer::PopAll()
{
	m_VB->Bind();
	m_VB->Empty();
	m_Triangles = 0;
	m_SpritesOnScreen.clear();

	RefreshVertBuffer();
}

void Minecraft::Helper::SpriteRenderer::PopSprite(unsigned int id)
{
	for (int i = 0; i < m_SpritesOnScreen.size(); i++) {
		if (m_SpritesOnScreen[i]->Id == id) {
			m_SpritesOnScreen.erase(m_SpritesOnScreen.begin() + i);
		}
	}

	RefreshVertBuffer();
}

int Minecraft::Helper::SpriteRenderer::PushSprite(unsigned int id)
{
	if (m_Sprites.find(id) != m_Sprites.end()) {
		m_SpritesOnScreen.push_back(m_Sprites[id]);
		RefreshVertBuffer();
		return id;
	}
	else return -1;
}

unsigned int Minecraft::Helper::SpriteRenderer::PushSprite(const Sprite& sprite)
{
	return PushSprite(sprite.Path, sprite.Position, sprite.Size, sprite.Uvs, sprite.FlipUvs);
}

unsigned int Minecraft::Helper::SpriteRenderer::PushSprite(const std::string& path, const glm::vec2& position, const glm::vec2& size, Helper::Vec2_4 uvs, const bool flipUV)
{
	float index = -1.f;
	for (int i = 0; i < m_Samplers.size(); i++) {
		if (m_Samplers[i]->GetPath().compare(path) == 0) {
			index = (float)i;
		}
	}

	if (index == -1.f) {
		m_Samplers.push_back(new Texture(path, flipUV));
		index = m_Samplers.size() - 1.f;
	}

	Minecraft::Sprite2DVertex verts[4] = {
		{position + glm::vec2(0.f, 0.f), uvs.u0, index},
		{position + glm::vec2(size.x, 0.f), uvs.u1, index},
		{position + glm::vec2(size.x, size.y), uvs.u2, index},
		{position + glm::vec2(0.f, size.y), uvs.u3, index}
	};

	SpriteBlueprint* bp = new SpriteBlueprint(path, position, size, flipUV);
	bp->vertices[0] = verts[0];
	bp->vertices[1] = verts[1];
	bp->vertices[2] = verts[2];
	bp->vertices[3] = verts[3];
	bp->Id = n_IdPtr;

	m_Sprites[n_IdPtr] = bp;
	m_SpritesOnScreen.push_back(bp);

	RefreshVertBuffer();

	return n_IdPtr++;
}

unsigned int Minecraft::Helper::SpriteRenderer::PushSprite(const std::string& path, const glm::vec2& position, const glm::vec2& size, const bool flipUV)
{
	float index = -1.f;
	for (int i = 0; i < m_Samplers.size(); i++) {
		if (m_Samplers[i]->GetPath().compare(path) == 0) {
			index = (float)i;
		}
	}

	if (index == -1.f) {
		m_Samplers.push_back(new Texture(path, flipUV));
		index = m_Samplers.size() - 1.f;
	}

	Minecraft::Sprite2DVertex verts[4] = {
		{position + glm::vec2(0.f, 0.f), {0.f, 1.f}, index},
		{position + glm::vec2(size.x, 0.f), {1.f, 1.f}, index},
		{position + glm::vec2(size.x, size.y), {1.f, 0.f}, index},
		{position + glm::vec2(0.f, size.y), {0.f, 0.f}, index}
	};

	SpriteBlueprint* bp = new SpriteBlueprint(path, position, size, flipUV);
	bp->vertices[0] = verts[0];
	bp->vertices[1] = verts[1];
	bp->vertices[2] = verts[2];
	bp->vertices[3] = verts[3];
	bp->Id = n_IdPtr;

	m_Sprites[n_IdPtr] = bp;
	m_SpritesOnScreen.push_back(bp);

	RefreshVertBuffer();

	return n_IdPtr++;
}

void Minecraft::Helper::SpriteRenderer::TransformSprite(const Helper::Vec2_4& transform, unsigned int id)
{
	SpriteBlueprint* bp = m_Sprites[id];
	bp->vertices[0].Position = transform.u0;
	bp->vertices[1].Position = transform.u1;
	bp->vertices[2].Position = transform.u2;
	bp->vertices[3].Position = transform.u3;

	RefreshVertBuffer();
}

void Minecraft::Helper::SpriteRenderer::TransformSprite(const glm::vec2& direction, unsigned int id)
{
	SpriteBlueprint* bp = m_Sprites[id];
	bp->vertices[0].Position += direction;
	bp->vertices[1].Position += direction;
	bp->vertices[2].Position += direction;
	bp->vertices[3].Position += direction;

	RefreshVertBuffer();
}

void Minecraft::Helper::SpriteRenderer::SetSpritePosition(const glm::vec2& position, unsigned int id)
{
	SpriteBlueprint* bp = m_Sprites[id];

	int h = abs(bp->vertices[0].Position.y - bp->vertices[3].Position.y), w = abs(bp->vertices[0].Position.x - bp->vertices[1].Position.x);

	bp->vertices[0].Position = position;
	bp->vertices[1].Position = { position.x + w, position.y + 0.f };
	bp->vertices[2].Position = { position.x + w, position.y + h };
	bp->vertices[3].Position = { position.x + 0.f, position.y + h };

	RefreshVertBuffer();
}

void Minecraft::Helper::SpriteRenderer::DeleteSprite(unsigned int id)
{
	PopSprite(id);
	delete m_Sprites[id];
}

void Minecraft::Helper::SpriteRenderer::DeleteAll()
{
	PopAll();
	for (auto it = m_Sprites.begin(); it != m_Sprites.end(); it++)
	{
		delete it->second;
	}
	m_Sprites.clear();
}

void Minecraft::Helper::SpriteRenderer::RefreshVertBuffer()
{
	m_VB->Bind();
	m_VB->Empty();
	m_Triangles = m_SpritesOnScreen.size() * 2;

	for (SpriteBlueprint* bp : m_SpritesOnScreen) {
		m_VB->AddVertexData(bp->vertices, sizeof(Minecraft::Sprite2DVertex) * 4);
	}

	UpdateSamplerArray();
}

void Minecraft::Helper::SpriteRenderer::UpdateSamplerArray()
{
	int sampler[8]{};
	for (int i = 0; i < m_Samplers.size(); i++) {
		int j = m_Samplers[i]->GetBoundPort();
		if (j != -1) {
			sampler[i] = j;
		}
		else {
			if (m_SamplerPtr + 1 > m_SamplerRangeHigh) {
				LOGC("Error: Trying to bind a texture outside of defined sampler range!", LOG_COLOR::FAULT);
				m_SamplerPtr = 0;
			}
			sampler[i] = m_Samplers[i]->Bind(Minecraft::Global::SAMPLER_SLOT_SPRITES + m_SamplerRangeLow + m_SamplerPtr);
		}
	}

	m_Shader->Bind();
	m_Shader->SetUniform1iv("u_Textures", 8, sampler);
}