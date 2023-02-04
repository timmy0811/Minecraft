#include "Handler.h"

Handler::Handler(GLFWwindow* window)
	:r_Window(window), m_World(window)
{
	// GLFW Input Mode configuration
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GL Flags
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glEnable(GL_BLEND));
	GLCall(glEnable(GL_DEPTH_TEST));
}

void Handler::OnInput(GLFWwindow* window)
{
	m_World.OnInput(window, v_DeltaTime);
}

void Handler::OnRender()
{
	m_World.OnRender();
}

void Handler::OnUpdate()
{
	m_World.OnUpdate();

	float currentFrame = (float)glfwGetTime();
	v_DeltaTime = currentFrame - v_LastFrame;
	v_LastFrame = currentFrame;
}
