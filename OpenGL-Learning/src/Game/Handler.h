#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include "world/World.h"
#include "view/Skybox.h"

#include "Game/render/CustomRenderer.h"
#include "windowsWrapper.h"
#include "Game/application/TexturePacker.h"

class Handler
{
private:
	World m_World;
	Skybox m_Skybox;
	Minecraft::Helper::FontRenderer m_FontRenderer;

	// Application
	TexturePacker Packer;

	// Time 
	float v_DeltaTime;
	float v_LastFrame;

	GLFWwindow* r_Window;

	void OnInit();
	void DebugWindow();

public:
	Handler(GLFWwindow* window);

	void OnInput(GLFWwindow* window);
	void OnRender();
	void OnUpdate();
};

