#pragma once

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include "glm/gtc/matrix_transform.hpp"

#include <vector>
#include <map>

#include "Chunk.h"
#include "../View/Camera.h"
#include "vendor/yaml/yaml_wrapper.hpp"

#include "OpenGL_util/texture/Texture.h"
#include "OpenGL_util/core/Shader.h"

class World
{
private:
	glm::mat4 m_MatrixProjection;
	glm::mat4 m_MatrixView;
	glm::mat4 m_MatrixTranslation;

	// Noise
	siv::PerlinNoise::seed_type m_NoiseSeed = 123456u;
	siv::PerlinNoise m_Noise;

	// Objects
	Minecraft::Camera3D m_Camera;
	std::vector<Chunk*> m_Chunks;

	std::map<unsigned int, Minecraft::Block_format> m_BlockFormats;
	std::map<const std::string, Minecraft::Texture_Format> m_TextureFormats;

	void ParseBlocks(const std::string& path);
	void ParseTextures(const std::string& path);

	// Textures
	Texture m_TextureMap;

	// Shader 
	Minecraft::Helper::ShaderPackage m_ShaderPackage;

	// Camera Handling
	GLFWwindow* r_Window;

	float m_LastX = 400, m_LastY = 300;
	bool m_FirstInit = true;

	static void OnMouseCallback(GLFWwindow* window, double xpos, double ypos);
	void ProcessMouse();

	// Generation
	void GenerateTerrain();

public:
	World(GLFWwindow* window);
	~World();

	void OnInput(GLFWwindow* window, double deltaTime);
	void OnRender();
	void OnUpdate();

	// Members
	inline static float s_MouseX = 0;
	inline static float s_MouseY = 0;
};
