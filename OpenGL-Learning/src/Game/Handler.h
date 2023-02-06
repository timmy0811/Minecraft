#pragma once

#include "world/World.h"
#include "view/Skybox.h"

#include "Game/application/TexturePacker.h"

class Handler
{
private:
	World m_World;
	Skybox m_Skybox;

	// Time 
	float v_DeltaTime;
	float v_LastFrame;

	GLFWwindow* r_Window;

	void OnInit();

public:
	Handler(GLFWwindow* window);

	void OnInput(GLFWwindow* window);
	void OnRender();
	void OnUpdate();
};

