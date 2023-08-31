#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <glm/glm.hpp>
#include <GLEW/glew.h>

#include "../debug/Debug.hpp"

#define MAX_BUF 8

enum BufferDataType {
	_FLOAT = 0x1406,
	_FLOAT16 = 1,
	_DOUBLE = 0x140A,
	_BYTE_UNSIGNED = 0x1401,
	_BYTE = 0x1400,
	_SHORT = 0x1402,
	_SHORT_UNSIGNED = 0x1403,
	_INT_UNSIGNED = 0x1405,
	_INT = 0x1404
};

enum BufferFormat {
	RGBA32F = 0x8814,
	RGB32F = 0x8815,
	RG32F = 0x8230,
	R32F = 0x822E,

	RGBA16F = 0x881A,
	RGB16F = 0x881B,
	RG16F = 0x822F,
	R16F = 0x822D,

	RGBA32UI = 0x8D70,
	RGB32UI = 0x8D71,
	RG32UI = 0x823C,
	R32UI = 0x8236,

	RGBA16UI = 0x8D76,
	RGB16UI = 0x8D77,
	RG16UI = 0x823A,
	R16UI = 0x8234,

	RGBA8UI = 0x8D7C,
	RGB8UI = 0x8D7D,
	RG8UI = 0x8238,
	R8UI = 0x8232,

	RGBA32I = 0x8D82,
	RGB32I = 0x8D83,
	RG32I = 0x823B,
	R32I = 0x8235,

	RGBA16I = 0x8D88,
	RGB16I = 0x8D89,
	RG16I = 0x8239,
	R16I = 0x8233,

	RGBA8I = 0x8D8E,
	RGB8I = 0x8D8F,
	RG8I = 0x8237,
	R8I = 0x8231,

	RGBA = GL_RGBA,
	RGB = GL_RGB,
	RG = GL_RG,
	R = GL_R
};

enum class DepthBufferType { WRITE_ONLY, WRITE_READ };

class Framebuffer
{
public:
	Framebuffer(const glm::ivec2& size, bool attachDepth = true, DepthBufferType depthType = DepthBufferType::WRITE_ONLY);
	~Framebuffer();

	static void Bind(unsigned int Framebuffer);

	void Bind() const;
	void BindAndClear();
	void Unbind();

	void BindTextures(const unsigned int startSlot = 0);
	void BindTexture(int index, const unsigned int startSlot = 0);
	unsigned int BindDepthTexture(const unsigned int slot);

	bool PushColorAttribute(const char channel = 3, BufferDataType dataType = BufferDataType::_FLOAT, const void* data = nullptr);
	bool PushColorAttribute(unsigned int internalFormat = GL_RGBA16F, unsigned int format = GL_RGBA, unsigned int dataType = GL_FLOAT, const void* data = nullptr);

	inline size_t AttachementCount() const { return m_Attachements.size(); }
	inline bool Validate() const;
	inline void Resize(const glm::ivec2& size) { m_Width = size.x; m_Height = size.y; }

private:
	unsigned int m_Width, m_Height;
	unsigned int m_IdFBO, m_RBODepth;
	std::vector<GLuint> m_Buffers;
	std::vector<GLuint> m_Attachements;

	int m_BoundPort;
};

class GBufferStandard
{
public:
	GBufferStandard(const glm::ivec2& size);
	~GBufferStandard();

	void BindAndClear();
	void Unbind();

	void BindTextures(const unsigned int startSlot = 0);
	void BindTexturesGeo(const unsigned int startSlot = 0);

	inline void Resize(const glm::ivec2& size) { m_Width = size.x; m_Height = size.y; }

private:
	unsigned int m_Width, m_Height;
	unsigned int m_IdFBO, m_RBODepth;
	unsigned int m_IdPosition, m_IdNormal, m_IdAlbedo;

	int m_BoundPort;
};
