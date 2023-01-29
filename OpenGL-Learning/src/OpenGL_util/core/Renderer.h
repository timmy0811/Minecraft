#pragma once

#include <GLEW/glew.h>

#include "../debug/Debug.hpp"

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

class Renderer {
public:
    void Clear() const;
    static void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
};