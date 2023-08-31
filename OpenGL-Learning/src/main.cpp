#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "OpenGL_util/core/Renderer.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "config.h"
#include "window.h"

#include "Game/Handler.h"

#include "imgui_helper/imgui.h"

int main(void)
{
#ifndef _DEBUG
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
#endif

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	// Window hints for glfw
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, 1);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(conf.WIN_WIDTH_INIT, conf.WIN_HEIGHT_INIT, "Mineclone", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Icon
	GLFWimage images[1];
	images[0].pixels = stbi_load("res/images/icon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels
	glfwSetWindowIcon(window, 1, images);
	stbi_image_free(images[0].pixels);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cout << "Could not init glew." << std::endl;
		return -1;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Set Blending
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glEnable(GL_BLEND));

	ImGuiIO& io = ImGuiInit();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	// Game
	Handler GameHandler = Handler(window);
	GLContext::Init();

#define CLEAR_BUFFER_MAIN

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		pollResizeEvent(window);

		/* Render here */
#ifdef CLEAR_BUFFER_MAIN
		GLContext::Clear();
		GLCall(glClearColor(0.f, 0.f, 0.f, 1.f));
#endif // CLEAR_BUFFER_MAIN

		//Game
		GameHandler.OnInput(window);
		GameHandler.OnUpdate();
		GameHandler.OnRender();

		ImGuiNewFrame();
#ifdef _DEBUG
		GameHandler.OnRenderGUIDebug();
#endif // !_DEBUG

		ImGuiRender(io);

		// glfw handling
		GLCall(glfwSwapBuffers(window));
		GLCall(glfwPollEvents());
	}

	ImGuiShutdown();
	glfwTerminate();
	return 0;
}