#pragma once

#include <memory>
#include <string>
#include <iostream>

#include "config.h"

#include "../primitive/Primitive.hpp"

#include "OpenGL_util/core/Renderer.h"

#include "OpenGL_util/core/VertexBuffer.h"
#include "OpenGL_util/core/VertexArray.h"
#include "OpenGL_util/core/IndexBuffer.h"
#include "OpenGL_util/core/VertexBufferLayout.h"
#include "OpenGL_util/core/Shader.h"
#include "OpenGL_util/texture/Texture.h"

namespace Minecraft::Render {
	class LineRenderer {
	public:
		std::unique_ptr<VertexBuffer> vb;
		std::unique_ptr<IndexBuffer> ib;
		std::unique_ptr<VertexBufferLayout> vbLayout;
		std::unique_ptr<VertexArray> va;

		Shader* shader;

		LineRenderer(int count, const std::string& shaderVert, const std::string& shaderFrag)
		: shader(new Shader(shaderVert, shaderFrag)) {
			unsigned int* indices = new unsigned int[count];

			for (int i = 0; i < count; i++) {
				indices[i] = i;
			}

			ib = std::make_unique<IndexBuffer>(indices, count);
			vb = std::make_unique<VertexBuffer>(count, sizeof(Minecraft::PositionVertex));

			delete[] indices;

			vbLayout = std::make_unique<VertexBufferLayout>();
			vbLayout->Push<float>(3);	// Position

			va = std::make_unique<VertexArray>();
			va->AddBuffer(*vb, *vbLayout);

			shader->Bind();
			shader->SetUniform4f("u_Color", conf.CHUNK_BORDER_COLOR.r, conf.CHUNK_BORDER_COLOR.g, conf.CHUNK_BORDER_COLOR.b, conf.CHUNK_BORDER_COLOR.a);
		}

		~LineRenderer() {
			delete shader;
		}

		inline void Draw() {
			GLCall(glLineWidth(3));
			shader->Bind();
			va->Bind();
			ib->Bind();
			Renderer::Draw(*va, *ib, *shader, GL_LINES);
		}
	};

	class BlockSelectionRenderer {
	public:
		bool doDraw;

		std::unique_ptr<VertexBuffer> vb;
		std::unique_ptr<IndexBuffer> ib;
		std::unique_ptr<VertexBufferLayout> vbLayout;
		std::unique_ptr<VertexArray> va;

		Shader* shader;

		BlockSelectionRenderer(const std::string& shaderVert, const std::string& shaderFrag)
			: shader(new Shader(shaderVert, shaderFrag)) {
			unsigned int* indices = new unsigned int[24];

			for (int i = 0; i < 24; i++) {
				indices[i] = i;
			}

			ib = std::make_unique<IndexBuffer>(indices, 24);
			vb = std::make_unique<VertexBuffer>(24, sizeof(Minecraft::PositionVertex));

			delete[] indices;

			vbLayout = std::make_unique<VertexBufferLayout>();
			vbLayout->Push<float>(3);	// Position

			va = std::make_unique<VertexArray>();
			va->AddBuffer(*vb, *vbLayout);

			shader->Bind();
			shader->SetUniform4f("u_Color", 0.1f, 0.1f, 0.1f, 1.f);
		}

		~BlockSelectionRenderer() {
			delete shader;
		}

		void LoadOutlineBuffer(const glm::vec3& position, const float size) {
			vb->Empty();
			
			Minecraft::PositionVertex v[24];

			v[0].Position = { position.x + 0,		position.y + 0,		position.z + size };
			v[1].Position = { position.x + 0,		position.y + size,	position.z + size };
			v[2].Position = { position.x + 0,		position.y + 0,		position.z + 0 };
			v[3].Position = { position.x + 0,		position.y + size,	position.z + 0 };

			v[4].Position = { position.x + size,	position.y + 0,		position.z + 0 };
			v[5].Position = { position.x + size,	position.y + size,	position.z + 0 };
			v[6].Position = { position.x + size,	position.y + 0,		position.z + size };
			v[7].Position = { position.x + size,	position.y + size,	position.z + size };

			v[8].Position = { position.x + 0,		position.y + 0,		position.z + size };
			v[9].Position = { position.x + size,	position.y + 0,		position.z + size };
			v[10].Position = { position.x + size,	position.y + 0,		position.z + size };
			v[11].Position = { position.x + size,	position.y + 0,		position.z + 0 };

			v[12].Position = { position.x + size,	position.y + 0,		position.z + 0 };
			v[13].Position = { position.x + 0,		position.y + 0,		position.z + 0 };
			v[14].Position = { position.x + 0,		position.y + 0,		position.z + 0 };
			v[15].Position = { position.x + 0,		position.y + 0,		position.z + size };

			v[16].Position = { position.x + 0,		position.y + size,	position.z + size };
			v[17].Position = { position.x + size,	position.y + size,	position.z + size };
			v[18].Position = { position.x + size,	position.y + size,	position.z + size };
			v[19].Position = { position.x + size,	position.y + size,	position.z + 0 };

			v[20].Position = { position.x + size,	position.y + size,	position.z + 0 };
			v[21].Position = { position.x + 0,		position.y + size,	position.z + 0 };
			v[22].Position = { position.x + 0,		position.y + size,	position.z + 0 };
			v[23].Position = { position.x + 0,		position.y + size,	position.z + size };

			vb->AddVertexData(v, sizeof(Minecraft::PositionVertex) * 24, 0);
			doDraw = true;
		}

		inline void DoNotDraw() { doDraw = false; };

		inline void Draw() {
			if (!doDraw) return;

			GLCall(glLineWidth(3));
			shader->Bind();
			va->Bind();
			ib->Bind();
			Renderer::Draw(*va, *ib, *shader, GL_LINES);
		}
	};

	class SpriteRenderer {
	public:
		std::unique_ptr<VertexBuffer> vb;
		std::unique_ptr<IndexBuffer> ib;
		std::unique_ptr<VertexBufferLayout> vbLayout;
		std::unique_ptr<VertexArray> va;

		glm::mat4 projection;
		glm::mat4 view;
		glm::vec3 translation;

		Shader* shader;
		size_t triangles = 0;

		std::vector<Texture*> textures;

		SpriteRenderer(int maxSprites, const std::string& shaderVert, const std::string& shaderFrag)
			: shader(new Shader(shaderVert, shaderFrag)) {
			projection = glm::ortho(0.0f, (float)conf.WIN_WIDTH, 0.0f, (float)conf.WIN_HEIGHT, -1.0f, 1.0f);
			translation = glm::vec3(0.f, 0.f, 0.f);
			view = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f));

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

			ib = std::make_unique<IndexBuffer>(indices, maxSprites * 6);
			vb = std::make_unique<VertexBuffer>(maxSprites * 4, sizeof(Minecraft::Sprite2DVertex));

			delete[] indices;

			vbLayout = std::make_unique<VertexBufferLayout>();
			vbLayout->Push<float>(2);	// Position
			vbLayout->Push<float>(2);	// UVs
			vbLayout->Push<float>(1);	// Texture Index

			va = std::make_unique<VertexArray>();
			va->AddBuffer(*vb, *vbLayout);

			shader->Bind();
			shader->SetUniformMat4f("u_MVP", projection * view * glm::translate(glm::mat4(1.f), translation));

			// RefreshTextures();
		}

		void RefreshTextures() {
			int sampler[8]{};
			for (int i = 0; i < textures.size(); i++) {
				if (int j = textures[i]->GetBoundPort() != -1) {
					sampler[i] = j;
				}
				else {
					sampler[i] = textures[i]->Bind(8);
				}
			}

			shader->Bind();
			shader->SetUniform1iv("u_Textures", 8, sampler);
		}

		void Empty() {
			vb->Empty();
		}

		void AddSprite(const std::string& path, const glm::vec2& position, const glm::vec2& size, const bool flipUV = false) {
			float index = -1.f;
			for (int i = 0; i < textures.size(); i++) {
				if (textures[i]->GetPath().compare(path) == 0) {
					index = (float)i;
				}
			}

			if (index == -1.f) {
				textures.push_back(new Texture(path, flipUV));
				index = textures.size() - 1.f;
			}

			Minecraft::Sprite2DVertex verts[4] = {
				{position + glm::vec2(0.f, 0.f), {0.f, 1.f}, index},
				{position + glm::vec2(size.x, 0.f), {1.f, 1.f}, index},
				{position + glm::vec2(size.x, size.y), {1.f, 0.f}, index},
				{position + glm::vec2(0.f, size.y), {0.f, 0.f}, index}
			};

			vb->Bind();
			vb->AddVertexData(verts, sizeof(Minecraft::Sprite2DVertex) * 4);
			triangles += 2;

			RefreshTextures();
		}

		~SpriteRenderer() {
			delete shader;

			for (Texture* tex : textures) {
				delete tex;
			}
		}

		inline void Draw() {
			shader->Bind();
			va->Bind();
			ib->Bind();
			Renderer::Draw(*va, *ib, *shader, GL_TRIANGLES, ib->GetCount());
			shader->Unbind();
		}
	};
}