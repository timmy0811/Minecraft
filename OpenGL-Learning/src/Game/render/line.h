#pragma once

#include <memory>
#include "config.h"
#include "../primitive/Primitive.hpp"

#include "OpenGL_util/core/VertexBuffer.h"
#include "OpenGL_util/core/VertexArray.h"
#include "OpenGL_util/core/IndexBuffer.h"
#include "OpenGL_util/core/VertexBufferLayout.h"
#include "OpenGL_util/core/Shader.h"

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
			vb = std::make_unique<VertexBuffer>(count, sizeof(Minecraft::LineVertex));

			delete[] indices;

			vbLayout = std::make_unique<VertexBufferLayout>();
			vbLayout->Push<float>(3);	// Position
			vbLayout->Push<float>(3);	// Color

			va = std::make_unique<VertexArray>();
			va->AddBuffer(*vb, *vbLayout);
		}

		~LineRenderer() {
			delete shader;
		}

		inline void Draw() {
			shader->Bind();
			va->Bind();
			ib->Bind();
			Renderer::Draw(*va, *ib, *shader, GL_LINES);
		}
	};
}