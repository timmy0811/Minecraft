#pragma once

#include <memory>
#include <map>
#include <string>
#include <iostream>

#include "config.h"

#include "glm/glm.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "Game/primitive/Primitive.hpp"
#include "Game/render/Helper.h"

#include "OpenGL_util/core/Renderer.h"

#include "OpenGL_util/core/VertexBuffer.h"
#include "OpenGL_util/core/VertexArray.h"
#include "OpenGL_util/core/IndexBuffer.h"
#include "OpenGL_util/core/VertexBufferLayout.h"
#include "OpenGL_util/core/Shader.h"
#include "OpenGL_util/texture/Texture.h"

namespace Minecraft::Helper {
	class SpriteRenderer {
	public:
		SpriteRenderer(int maxSprites, glm::uvec2 samplerSlotRange, const std::string& shaderVert, const std::string& shaderFrag);
		~SpriteRenderer();

		void Draw() const;

		void PushAll();
		void PopAll();
		void PopSprite(unsigned int id);

		int PushSprite(unsigned int id);
		unsigned int PushSprite(const Sprite& sprite);
		unsigned int PushSprite(const std::string& path, const glm::vec2& position, const glm::vec2& size, Helper::Vec2_4 uvs, const bool flipUV = false);
		unsigned int PushSprite(const std::string& path, const glm::vec2& position, const glm::vec2& size, const bool flipUV = false);

		void TransformSprite(const Helper::Vec2_4& uvs, unsigned int id);
		void TransformSprite(const glm::vec2& direction, unsigned int id);
		void SetSpritePosition(const glm::vec2& position, unsigned int id);

		void DeleteSprite(unsigned int id);
		void DeleteAll();

	private:
		void RefreshVertBuffer();
		void UpdateSamplerArray();

	private:
		std::unique_ptr<VertexBuffer> m_VB;
		std::unique_ptr<IndexBuffer> m_IB;
		std::unique_ptr<VertexBufferLayout> m_VBLayout;
		std::unique_ptr<VertexArray> m_VA;

		glm::mat4 m_MatProjection;
		glm::mat4 m_MatView;
		glm::vec3 m_MatTranslation;

		unsigned int m_SamplerRangeLow;
		unsigned int m_SamplerRangeHigh;

		Shader* m_Shader;
		size_t m_Triangles = 0;

		std::vector<Texture*> m_Samplers;
		std::map<unsigned int, SpriteBlueprint*> m_Sprites;
		std::vector< SpriteBlueprint*> m_SpritesOnScreen;

		unsigned int n_IdPtr;
		size_t m_SamplerPtr;
	};
}
