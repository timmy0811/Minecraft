#pragma once

#include <iostream>

#include "OpenGL_util/core/VertexBuffer.h"
#include "OpenGL_util/core/VertexArray.h"
#include "OpenGL_util/core/VertexBufferLayout.h"

#include "OpenGL_util/core/Shader.h"
#include "OpenGL_util/core/Renderer.h"

#include "stb_image/stb_image.h"

class Skybox
{
private:
	mutable int m_BoundID;
	unsigned int m_RendererID;
	std::string m_FilepathPrefix;

	Shader m_Shader;

	// Cube object
	std::unique_ptr<VertexArray> m_VA;
	std::unique_ptr<VertexBuffer> m_VB;
	std::unique_ptr<VertexBufferLayout> m_VBLayout;

	// Debug
	unsigned int m_DrawCalls = 0;

public:
	Skybox(const std::string& cubemapPathPrefix, const std::string& fileFormat = ".png");
	~Skybox();

	void Unbind();
	const unsigned int Bind(const unsigned int slot);
	void OnRender();

	void setMatrix(const glm::mat4& projection, const glm::mat4& view);

	// Members
	inline const unsigned int getDrawCalls() const { return m_DrawCalls; }

};

