#pragma once

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include "glm/gtc/matrix_transform.hpp"

#include <vector>
#include <map>
#include <set>
#include <time.h>
#include <iostream>

#include "Chunk.h"
#include "../View/Camera.h"
#include "vendor/yaml/yaml_wrapper.hpp"

#include "OpenGL_util/texture/Texture.h"
#include "OpenGL_util/core/Shader.h"
#include "OpenGL_util/misc/Light.hpp"

class World
{
private:
	glm::mat4 m_MatrixProjection;
	glm::mat4 m_MatrixView;
	glm::mat4 m_MatrixTranslation;

	// Debug
	unsigned int m_DrawCalls = 0;

	// Noise
	siv::PerlinNoise m_Noise;

	// Objects
	Minecraft::Camera3D m_Camera;
	std::vector<Chunk*> m_Chunks;

	std::map<unsigned int, Minecraft::Block_format> m_BlockFormats;
	std::map<const std::string, Minecraft::Texture_Format> m_TextureFormats;
	std::set<std::string> m_UsedTextures;

	void ParseBlocks(const std::string& path);
	void ParseTextures(const std::string& path);

	// Textures
	Texture m_TextureMap;

	// Light
	OpenGL::DirectionalLight m_DirLight;

	void SetupLight();
	void updateLight();
	
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
	void OnUpdate(double deltaTime);

	// Members
	inline static float s_MouseX = 0;
	inline static float s_MouseY = 0;

	const glm::mat4& getMatrixProjection() const;
	const glm::mat4& getMatrixView() const;

	const unsigned int getAmountBlockStatic() const;
	inline const size_t getAmountChunk() const { return m_Chunks.size(); }
	inline const size_t getAmountBlockFormat() const { return m_BlockFormats.size(); };
	inline const size_t getAmountTextureFormat() const { return m_TextureFormats.size(); }
	const size_t getDrawnVertices() const;

	const unsigned int getDrawCalls() const;
	inline const Minecraft::Helper::ShaderPackage& getShaderPackage() { return m_ShaderPackage; };
};
