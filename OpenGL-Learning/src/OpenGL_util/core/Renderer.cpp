#include "Renderer.h"
#include <iostream>

void Renderer::Clear() 
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader)
{
    shader.Bind();
    va.Bind();
    ib.Bind();
    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::DrawArray(const VertexArray& va, const Shader& shader, size_t first, size_t count)
{
    shader.Bind();
    va.Bind();
    glDrawArrays(GL_TRIANGLES, (int)first, (int)count);
}
