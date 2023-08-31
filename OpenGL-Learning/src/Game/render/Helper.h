#pragma once

#include <glm/glm.hpp>

#include "OpenGL_util/core/Shader.h"
#include "OpenGL_util/core/Renderer.h"

namespace Minecraft::Helper {
	// Structs
	struct Vec2_4 {
		glm::vec2 u0;
		glm::vec2 u1;
		glm::vec2 u2;
		glm::vec2 u3;
	};

	struct SymbolInformation {
		Vec2_4 uv;
		unsigned int width;
	};

	struct ShaderPackage {
		Shader* shaderWorld;
		Shader* shaderShadowGeneration;
		Shader* shaderGBuffer;

		~ShaderPackage() {
			delete shaderWorld;
			delete shaderShadowGeneration;
			delete shaderGBuffer;
		}
	};

	struct ScreenQuad {
		ScreenQuad() 
		{
			m_MatProjectionVertex = glm::ortho(0.0f, (float)conf.WIN_WIDTH_INIT, 0.0f, (float)conf.WIN_HEIGHT_INIT, -1.0f, 1.0f);

			unsigned int index[] = {
				0, 1, 2, 2, 3, 0
			};

			m_IndexBuffer = std::make_unique<IndexBuffer>(index, 6);
			m_VertexBuffer = std::make_unique<VertexBuffer>(4, sizeof(Minecraft::PositionVertex));

			Minecraft::PositionVertex vert[] = {
				glm::vec3(-1.f, -1.f, 0.f),
				glm::vec3(1.f, -1.f, 0.f),
				glm::vec3(1.f, 1.f, 0.f),
				glm::vec3(-1.f, 1.f, 0.f)
			};

			m_VertexBuffer->Bind();
			m_VertexBuffer->AddVertexData(vert, sizeof(Minecraft::PositionVertex) * 4);

			m_VertexBufferLayout = std::make_unique<VertexBufferLayout>();
			m_VertexBufferLayout->Push<float>(3);	// Position

			m_VertexArray = std::make_unique<VertexArray>();
			m_VertexArray->AddBuffer(*m_VertexBuffer, *m_VertexBufferLayout);
		}

		void Draw(const Shader& shader) {
			GLContext::Draw(*m_VertexArray, *m_IndexBuffer, shader, GL_TRIANGLES, 6);
		}

		// Only used for custom dimensions
		void SetProjectionMat(Shader& shader) {
			shader.Bind();
			shader.SetUniformMat4f("u_ScreenQuadProj", m_MatProjectionVertex);
		}

		std::unique_ptr<VertexBuffer> m_VertexBuffer;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
		std::unique_ptr<VertexBufferLayout> m_VertexBufferLayout;
		std::unique_ptr<VertexArray> m_VertexArray;

		glm::mat4 m_MatProjectionVertex;
	};

	struct Sprite {
		Sprite(const std::string& path, const glm::vec2& position, const glm::vec2& size, const bool flipUV = false) {
			this->Path = path;
			this->Position = position;
			this->Size = size;
			this->FlipUvs = flipUV;

			Uvs.u0 = { 0.f, 1.f };
			Uvs.u1 = { 1.f, 1.f };
			Uvs.u2 = { 1.f, 0.f };
			Uvs.u3 = { 0.f, 0.f };
		}

		Sprite(const std::string& path, const glm::vec2& position, const glm::vec2& size, Helper::Vec2_4 uvs, const bool flipUV = false) {
			this->Path = path;
			this->Position = position;
			this->Size = size;
			this->FlipUvs = flipUV;

			Uvs = uvs;
		}

		Sprite() {};

		unsigned int Id;
		std::string Path;
		glm::vec2 Position;
		glm::vec2 Size;
		Helper::Vec2_4 Uvs;

		bool FlipUvs;
	};

	struct SpriteBlueprint : public Sprite {
		Minecraft::Sprite2DVertex vertices[4];

		SpriteBlueprint(const std::string& path, const glm::vec2& position, const glm::vec2& size, const bool flipUV = false) :Sprite(path, position, size, flipUV) {}
		SpriteBlueprint(const std::string& path, const glm::vec2& position, const glm::vec2& size, Helper::Vec2_4 uvs, const bool flipUV = false) :Sprite(path, position, size, uvs, flipUV) {}
	};

	// Functions
	static int mapRGBToInt(const glm::vec3& color)
	{
		int r = int(color.r * 255) << 16;
		int g = int(color.g * 255) << 8;
		int b = int(color.b * 255);

		return r | g | b;
	}
}