#pragma once

#include "world/World.h"
#include "Game/application/TexturePacker.h"

class Handler
{
private:
	World m_World;

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

