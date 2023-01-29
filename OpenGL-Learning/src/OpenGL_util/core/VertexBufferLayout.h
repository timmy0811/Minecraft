#pragma once

#include <vector>
#include <GLEW/glew.h>

#include "../debug/Debug.hpp"

struct VertexBufferElement {
	unsigned int type;
	unsigned int count;
	unsigned char normalised;

	static unsigned int GetSize(unsigned int type) {
		switch (type) {
		case GL_FLOAT: return 4;
		case GL_UNSIGNED_INT: return 4;
		case GL_INT: return 4;
		case GL_UNSIGNED_BYTE: return 1;
		}
		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_Elements;
	unsigned int m_Stride;

public:
	VertexBufferLayout()
		:m_Stride(0){}

	template<typename T>
	void Push(unsigned int count) {
	}

	template<>
	void Push<float>(unsigned int count) {
		VertexBufferElement element{ GL_FLOAT, count, GL_FALSE };
		m_Elements.push_back(element);
		m_Stride += VertexBufferElement::GetSize(GL_FLOAT) * count;
	}

	template<>
	void Push<unsigned int>(unsigned int count) {
		VertexBufferElement element{ GL_UNSIGNED_INT, count, GL_FALSE };
		m_Elements.push_back(element);
		m_Stride += VertexBufferElement::GetSize(GL_UNSIGNED_INT) * count;
	}

	template<>
	void Push<int>(unsigned int count) {
		VertexBufferElement element{ GL_INT, count, GL_FALSE };
		m_Elements.push_back(element);
		m_Stride += VertexBufferElement::GetSize(GL_INT) * count;
	}

	template<>
	void Push<unsigned char>(unsigned int count) {
		VertexBufferElement element{ GL_UNSIGNED_BYTE, count, GL_TRUE };
		m_Elements.push_back(element);
		m_Stride += VertexBufferElement::GetSize(GL_UNSIGNED_BYTE) * count;
	}

	inline const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
	inline unsigned int GetStride() const { return m_Stride; }
};

