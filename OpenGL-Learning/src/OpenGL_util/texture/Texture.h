#pragma once

#include "../core/Renderer.h"
#include "stb_image/stb_image.h"

enum class TextureType { DIFFUSE, SPECULAR, SHINE, NORMAL, HEIGHT, DEFAULT };

class Texture
{
private:
	mutable int m_BoundID;
	unsigned int m_RendererID;
	std::string m_Filepath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;

	TextureType m_Type;

public:
	Texture(const std::string& path);
	~Texture();

	void Bind(const unsigned int slot = 0) const;
	void Unbind();

	inline const std::string& GetPath() const { return m_Filepath; };
	inline int GetWidth() const { return m_Width; };
	inline int GetHeight() const { return m_Height; };
	inline int GetRendererID() const { return m_RendererID; };
	inline int GetBoundPort() const { return m_BoundID; };

	void SetType(TextureType type) { m_Type = type; };
	inline TextureType GetType() { return m_Type; };
};

