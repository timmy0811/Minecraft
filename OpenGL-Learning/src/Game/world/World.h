#pragma once

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include "glm/gtc/matrix_transform.hpp"

#include <vector>

#include "Chunk.h"
#include "../View/Camera.h"
#include "OpenGL_util/texture/Texture.h"

#include "OpenGL_util/core/Shader.h"

class World
{
private:
	glm::mat4 m_MatrixProjection;
	glm::mat4 m_MatrixView;
	glm::mat4 m_MatrixTranslation;

	// Objects
	Minecraft::Camera3D m_Camera;
	std::vector<Chunk> m_Chunks;

	// Textures
	Texture m_Texture_Log_Top;
	Texture m_Texture_Log_Side;

	// Shader 
	Minecraft::Helper::ShaderPackage m_ShaderPackage;

	// Camera Handling
	GLFWwindow* r_Window;

	float m_LastX = 400, m_LastY = 300;
	bool m_FirstInit = true;

	static void OnMouseCallback(GLFWwindow* window, double xpos, double ypos);
	void ProcessMouse();

public:
	World(GLFWwindow* window);

	void OnInput(GLFWwindow* window, double deltaTime);
	void OnRender();
	void OnUpdate();

	// Members
	inline static float s_MouseX = 0;
	inline static float s_MouseY = 0;
};
