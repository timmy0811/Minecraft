#include "ShadowMap.h"

ShadowMap::ShadowMap(const glm::ivec2& size)
{
	Init(size);
}

ShadowMap::~ShadowMap()
{
}

bool ShadowMap::Init(const glm::ivec2& size)
{
	m_Width = size.x;
	m_Height = size.y;

	GLCall(glGenFramebuffers(1, &m_IdFBO));

	GLCall(glGenTextures(1, &m_IdDepthBuffer));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_IdDepthBuffer));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));

#ifdef FILTER_SHADOW
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
#endif

#ifndef FILTER_SHADOW
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
#endif

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_IdFBO));
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_IdDepthBuffer, 0));

	GLCall(glDrawBuffer(GL_NONE));
	GLCall(glReadBuffer(GL_NONE));

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "[OpenGL Error] (" << std::to_string(status) << ")" << std::endl;
		return false;
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	return true;
}

void ShadowMap::BindAndClear()
{
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_IdFBO));
	GLCall(glViewport(0, 0, m_Width, m_Height));
	GLCall(glClear(GL_DEPTH_BUFFER_BIT));
}

unsigned int ShadowMap::BindDepthTexture(const unsigned int slot)
{
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_IdDepthBuffer));
	m_BoundPort = static_cast<int>(slot);
	return m_BoundPort;
}

void ShadowMap::UnbindDepthTexture()
{
	GLCall(glActiveTexture(m_BoundPort));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	m_BoundPort = -1;
}