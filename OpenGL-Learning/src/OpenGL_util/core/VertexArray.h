#pragma once

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

#include "../debug/Debug.hpp"

class VertexBufferLayout;

class VertexArray
{ 
private:
	unsigned int m_RendererID;

public:
	VertexArray();
	~VertexArray();

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
	void Bind() const;
	void Unbind() const;
};

