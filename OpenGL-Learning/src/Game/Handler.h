#pragma once

#include "world/World.h"

class Handler
{
private:
	World m_World;

	// Time 
	float v_DeltaTime;
	float v_LastFrame;

	GLFWwindow* r_Window;

public:
	Handler(GLFWwindow* window);

	void OnInput(GLFWwindow* window);
	void OnRender();
	void OnUpdate();
};

