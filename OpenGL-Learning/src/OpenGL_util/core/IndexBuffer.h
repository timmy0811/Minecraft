#pragma once

#include "../debug/Debug.hpp"

class IndexBuffer {
private:
	unsigned int m_RendererID;
	unsigned int m_Count;

public:
	IndexBuffer(const unsigned int* data, unsigned int count);
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;

	unsigned int GetCount() const;
};
