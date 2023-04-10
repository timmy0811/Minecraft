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
		std::map<unsigned int, Sprite> sprites;
		unsigned int idPtr;

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
		}

		void RefreshTextures() {
			int sampler[8]{};
			for (int i = 0; i < textures.size(); i++) {
				if (int j = textures[i]->GetBoundPort() != -1) {
					sampler[i] = j;
				}
				else {
					LOGC(std::to_string(SAMPLER_SLOT_SPRITES + Minecraft::Global::TEXTURE_BINDING));
					sampler[i] = textures[i]->Bind(SAMPLER_SLOT_SPRITES + Minecraft::Global::TEXTURE_BINDING++);		// TODO: Double check Index
				}
			}
			sampler[0] = 3;
			shader->Bind();
			shader->SetUniform1iv("u_Textures", 8, sampler);
		}

		void Empty() {
			vb->Empty();
		}

		unsigned int AddSprite(const std::string& path, const glm::vec2& position, const glm::vec2& size, const bool flipUV = false) {
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

			sprites[idPtr] = Sprite(path, position, size, flipUV);
			return idPtr++;
		}

		unsigned int AddSprite(const std::string& path, const glm::vec2& position, const glm::vec2& size, Helper::Vec2_4 uvs, const bool flipUV = false) {
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
				{position + glm::vec2(0.f, 0.f), uvs.u0, index},
				{position + glm::vec2(size.x, 0.f), uvs.u1, index},
				{position + glm::vec2(size.x, size.y), uvs.u2, index},
				{position + glm::vec2(0.f, size.y), uvs.u3, index}
			};

			vb->Bind();
			vb->AddVertexData(verts, sizeof(Minecraft::Sprite2DVertex) * 4);
			triangles += 2;

			RefreshTextures();
			sprites[idPtr] = Sprite(path, position, size, flipUV);
			return idPtr++;
		}

		unsigned int AddSprite(const Sprite& sprite) {
			return AddSprite(sprite.Path, sprite.Position, sprite.Size, sprite.Uvs, sprite.FlipUvs);
		}

		void RemoveSprite(unsigned int id) {
			sprites.erase(id);
			vb->Empty();

			for (auto const& [key, value] : sprites) {
				AddSprite(value);
			}
		}

		~SpriteRenderer() {
			delete shader;

			for (Texture* tex : textures) {
				delete tex;
			}
		}

		inline void Draw() {
			Renderer::Draw(*va, *ib, *shader, GL_TRIANGLES, ib->GetCount());
			shader->Unbind();
		}
	};

	class FontRenderer {
	private:
		Texture fontSheet;
		static inline std::map<int, Minecraft::Helper::SymbolInformation> symbols;

		unsigned int charHeight;
		int sheetHeight;
		int sheetWidth;
		int characterWidth;

		size_t count;
		bool unicode;

		std::unique_ptr<VertexBuffer> vb;
		std::unique_ptr<IndexBuffer> ib;
		std::unique_ptr<VertexBufferLayout> vbLayout;
		std::unique_ptr<VertexArray> va;

		glm::mat4 projection;
		glm::mat4 view;
		glm::vec3 translation;

		Shader* shader;

		Helper::SymbolInformation GatherSymbolInformation(int posX, int posY, int width) {
			SymbolInformation information{};

			glm::ivec2 imgPosition;
			imgPosition.x = posX * characterWidth + characterWidth / 2;
			imgPosition.y = posY * charHeight;

			if (unicode) {
				information.uv.u0.x = (imgPosition.x - characterWidth / 2.f) / sheetWidth;
				information.uv.u3.x = (imgPosition.x - characterWidth / 2.f) / sheetWidth;
				information.uv.u1.x = (imgPosition.x - characterWidth / 2.f + width) / sheetWidth;
				information.uv.u2.x = (imgPosition.x - characterWidth / 2.f + width) / sheetWidth;
			}
			else {
				information.uv.u0.x = (imgPosition.x - width / 2.f) / sheetWidth;
				information.uv.u3.x = (imgPosition.x - width / 2.f) / sheetWidth;
				information.uv.u1.x = (imgPosition.x + width / 2.f) / sheetWidth;
				information.uv.u2.x = (imgPosition.x + width / 2.f) / sheetWidth;
			}

			information.uv.u0.y = (float)imgPosition.y / (float)sheetHeight;
			information.uv.u1.y = (float)imgPosition.y / (float)sheetHeight;
			information.uv.u2.y = (imgPosition.y + charHeight) / (float)sheetHeight;
			information.uv.u3.y = (imgPosition.y + charHeight) / (float)sheetHeight;

			information.width = width;

			return information;
		}

	public:
		explicit FontRenderer(const std::string& imgPath, const std::string& fontPath, int capacity, const bool unicode = false)
		:fontSheet(imgPath, true), shader(new Shader("res/shaders/font/shader_font_stylized.vert", "res/shaders/font/shader_font_stylized.frag"))
		{
			projection = glm::ortho(0.0f, (float)conf.WIN_WIDTH, 0.0f, (float)conf.WIN_HEIGHT, -1.0f, 1.0f);
			translation = glm::vec3(0.f, 0.f, 0.f);
			view = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f));
			this->unicode = unicode;

			unsigned int* indices = new unsigned int[capacity * 6];

			unsigned int offset = 0;
			for (size_t i = 0; i < capacity * 6; i += 6) {
				indices[i + 0] = 0 + offset;
				indices[i + 1] = 1 + offset;
				indices[i + 2] = 2 + offset;

				indices[i + 3] = 2 + offset;
				indices[i + 4] = 3 + offset;
				indices[i + 5] = 0 + offset;

				offset += 4;
			}

			ib = std::make_unique<IndexBuffer>(indices, capacity * 6);
			vb = std::make_unique<VertexBuffer>(capacity * 4, sizeof(Minecraft::Sprite2DVertex));

			delete[] indices;

			vbLayout = std::make_unique<VertexBufferLayout>();
			vbLayout->Push<float>(2);	// Position
			vbLayout->Push<float>(2);	// UVs
			vbLayout->Push<float>(1);	// background
			vbLayout->Push<float>(1);	// alpha

			va = std::make_unique<VertexArray>();
			va->AddBuffer(*vb, *vbLayout);

			shader->Bind();
			shader->SetUniformMat4f("u_MVP", projection * view * glm::translate(glm::mat4(1.f), translation));

			fontSheet.Bind(SAMPLER_SLOT_FONTS);
			shader->SetUniform1i("u_FontSheetSampler", fontSheet.GetBoundPort());

			if(symbols.size() == 0) ParseSymbols(fontPath, this->unicode);
		}

		~FontRenderer() {
			delete shader;
		}

		void ParseSymbols(const std::string& fontPath, const bool unicode = false) {
			LOGC("Parsing Fonts", LOG_COLOR::SPECIAL_A);

			sheetHeight = fontSheet.GetHeight();
			sheetWidth = fontSheet.GetWidth();

			constexpr int charsPerRow = 16;
			charHeight = sheetHeight / charsPerRow;
			characterWidth = sheetWidth / charsPerRow;
			
			YAML::Node mainNode = YAML::LoadFile(fontPath);

			SymbolInformation informationUnknown = GatherSymbolInformation(
				mainNode["unknown"]["position"][0].as<int>(),
				mainNode["unknown"]["position"][1].as<int>(),
				mainNode["unknown"]["width"].as<unsigned int>()
			);
			symbols[255] = informationUnknown;

			for (auto symbol : mainNode["characters"]) {
				const char character = symbol.first.as<char>();

				int newWidth = symbol.second["width"].as<int>() / (255.f / sheetWidth);

				const SymbolInformation& information = GatherSymbolInformation(
					symbol.second["position"][0].as<int>(),
					symbol.second["position"][1].as<int>() - (unicode ? 2 : 0),
					newWidth
				);

				symbols[character] = information;
			}
		}

		void Clear() {
			vb->Empty();
		}

		void PrintMultilineText(const char* text, const glm::vec2& position, float size = 1.f, const glm::vec4& background = {}) {
			const char* symPtr = text;
			glm::vec2 symbolPosition = position;

			while (*symPtr != '\0') {
				const char symbol = *symPtr;
				if (symbol == '\n') {
					symbolPosition.x = position.x;
					symbolPosition.y -= (float)charHeight * size;

					symPtr++;
					continue;
				}
				Helper::SymbolInformation information;
				if (symbols.find(symbol) != symbols.end()) {
					information = symbols[symbol];
				}
				else {
					information = symbols[255];
				}

				AddLetter(information, symbolPosition, size, background);
				symbolPosition.x += information.width * size;

				symPtr++;
			}
		}

		void AddLetter(const Helper::SymbolInformation& information, const glm::vec2& position, float size = 1.f, const glm::vec4& background = {}) {
			SymbolVertex v[4];

			for (int i = 0; i < 4; i++) {
				v[i].background = (float)Helper::mapRGBToInt(background);
				v[i].alpha = background.a;
			}

			float w = information.width * size;
			float h = charHeight * size;

			v[0].Position = position;
			v[1].Position = position + glm::vec2(w, 0.f);
			v[2].Position = position + glm::vec2(w, h);
			v[3].Position = position + glm::vec2(0.f, h);

			v[0].uv = information.uv.u0;
			v[1].uv = information.uv.u1;
			v[2].uv = information.uv.u2;
			v[3].uv = information.uv.u3;

			vb->AddVertexData(v, sizeof(SymbolVertex) * 4);
			size++;
		}

		inline void Draw() {
			shader->Bind();
			Renderer::Draw(*va, *ib, *shader, GL_TRIANGLES, ib->GetCount());
			shader->Unbind();
		}

		inline const size_t getCount() const { return count; }
	};
}