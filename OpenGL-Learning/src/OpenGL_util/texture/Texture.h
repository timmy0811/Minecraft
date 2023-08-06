#pragma once

#include "../core/Renderer.h"
#include "vendor/stb_image/stb_image.h"

enum class TextureType { DIFFUSE, SPECULAR, SHINE, NORMAL, HEIGHT, DEFAULT };

class Texture
{
private:
	int m_BoundID = -1;
	unsigned int m_RendererID;
	std::string m_Filepath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;

	TextureType m_Type;

public:
	Texture(const std::string& path, const bool flipUV);
	~Texture();

	int Bind(const unsigned int slot = 0);
	void Unbind();

	inline const std::string& GetPath() const { return m_Filepath; };
	inline int GetWidth() const { return m_Width; };
	inline int GetHeight() const { return m_Height; };
	inline int GetRendererID() const { return m_RendererID; };
	inline int GetBoundPort() const { return m_BoundID; };

	void SetType(TextureType type) { m_Type = type; };
	inline TextureType GetType() { return m_Type; };
};
