#pragma once

#include <GLEW/glew.h>
#include <iostream>

// #define ASSERT(x) if(!(x)) __debugbreak();

#ifdef DEBUG
#define GLCall(x) GLCLearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#else
#define GLCall(x) x
#endif

void inline GLCLearError() {
    while (glGetError() != GL_NO_ERROR) {
    }
}

bool inline GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ": line " << line << std::endl;
        return false;
    }
    return true;
}
