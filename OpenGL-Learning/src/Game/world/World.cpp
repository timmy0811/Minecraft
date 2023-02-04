#include "World.h"

World::World(GLFWwindow* window)
	:m_ShaderPackage{ new Shader("res/shaders/block/static/shader_static.vert", "res/shaders/block/static/shader_static.frag") },
	r_Window(window),
	m_Texture_Log_Side("res/images/block/oak_log.png"), m_Texture_Log_Top("res/images/block/oak_log_top.png")
{
	m_MatrixView = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -3.5f));
	m_MatrixProjection = glm::perspective(glm::radians(45.f), (float)c_win_Width / (float)c_win_Height, 0.1f, 100.f);

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformMat4f("u_Projection", m_MatrixProjection);

	// Experimental Texture Setup
	m_Texture_Log_Side.Bind(0);
	m_Texture_Log_Top.Bind(1);

	int sampler[2] = { m_Texture_Log_Side.GetBoundPort(), m_Texture_Log_Top.GetBoundPort() };

	m_ShaderPackage.shaderBlockStatic->SetUniform1iv("u_Textures", 2, sampler);

	// Generate one chunk
	m_Chunks.push_back(Chunk());

	m_Chunks[0].Generate();
}

void World::OnRender()
{
	for (Chunk& chunk : m_Chunks) {
		chunk.OnRender(m_ShaderPackage);
	}
}

void World::OnUpdate()
{
	glfwSetCursorPosCallback(r_Window, OnMouseCallback);				// Camera not yet working

	ProcessMouse();
	m_MatrixView = glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Front, m_Camera.Up);

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformMat4f("u_View", m_MatrixView);
	m_ShaderPackage.shaderBlockStatic->SetUniform3f("u_ViewPosition", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);
}

void World::OnInput(GLFWwindow* window, double deltaTime)
{
	const float cameraSpeed = 5.f * (float)deltaTime; // adjust accordingly

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_Camera.Position += cameraSpeed * m_Camera.Front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_Camera.Position -= cameraSpeed * m_Camera.Front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_Camera.Position -= glm::normalize(glm::cross(m_Camera.Front, m_Camera.Up)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_Camera.Position += glm::normalize(glm::cross(m_Camera.Front, m_Camera.Up)) * cameraSpeed;
}

void World::ProcessMouse()
{
	if (m_FirstInit) {
		m_LastX = s_MouseX;
		m_LastY = s_MouseY;
		m_FirstInit = false;
	}

	float xoffset = s_MouseX - m_LastX;
	float yoffset = m_LastY - s_MouseY; // reversed since y-coordinates range from bottom to top
	m_LastX = s_MouseX;
	m_LastY = s_MouseY;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	m_Camera.yaw += xoffset;
	m_Camera.pitch += yoffset;

	if (m_Camera.pitch > 89.0f)
		m_Camera.pitch = 89.0f;
	if (m_Camera.pitch < -89.0f)
		m_Camera.pitch = -89.0f;

	glm::vec3 direction;

	direction.x = cos(glm::radians(m_Camera.yaw)) * cos(glm::radians(m_Camera.pitch));
	direction.y = sin(glm::radians(m_Camera.pitch));
	direction.z = sin(glm::radians(m_Camera.yaw)) * cos(glm::radians(m_Camera.pitch));
	m_Camera.Front = glm::normalize(direction);
}

void World::OnMouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	World::s_MouseX = float(xpos);
	World::s_MouseY = float(ypos);
}

