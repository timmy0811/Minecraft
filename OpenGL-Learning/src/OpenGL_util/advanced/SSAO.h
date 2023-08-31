#pragma once

#include <vector>
#include "OpenGL_util/buffer/GBuffer.h"

class SSAO
{
public:
	SSAO();

	void GenerateSampleKernel(int samples = 64);
	void GenerateSSAONoiseMap();
	glm::vec3* getKernelAllocator() { return &(m_Kernel[0]); }
	void BindNoiseTex(const unsigned int slot);

private:
	std::vector<glm::vec3> m_Kernel;
	unsigned int m_IdNoise;
};
