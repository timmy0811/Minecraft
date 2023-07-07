#pragma once

#include "GLFW/glfw3.h"
#include "GLEW/glew.h"

#include "config.h"

void OnWindowResize() {
	// Notify observers ...
	LOGC("Resized", LOG_COLOR::LOG);
	Minecraft::Global::updateResize = true;

	glViewport(0, 0, Minecraft::Global::windowSize.x, Minecraft::Global::windowSize.y);
}

bool pollResizeEvent(GLFWwindow* window) {
	static bool init = true;
	if (init) {
		Minecraft::Global::windowSize = { conf.WIN_WIDTH, conf.WIN_HEIGHT };
		init = false;
	}

	Minecraft::Global::updateResize = false;
	int x, y;
	glfwGetWindowSize(window, &x, &y);

	if (x != Minecraft::Global::windowSize.x || y != Minecraft::Global::windowSize.y) {
		Minecraft::Global::windowSize = { x, y };

		OnWindowResize();
		return true;
	}

	return false;
}