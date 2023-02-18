
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

#include "Game/Handler.h"

#include "imgui_helper/imgui.h"

#include "Game/application/TexturePacker.h"

int main(void)
{
    TexturePacker Packer{};

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // Window hints for glfw
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(c_win_Width, c_win_Height, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

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

    Renderer renderer;

    ImGuiIO& io = ImGuiInit();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Game
    Handler GameHandler = Handler(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        renderer.Clear();
        GLCall(glClearColor(0.f, 0.f, 0.f, 1.f));

        ImGuiNewFrame();

        //Game
        GameHandler.OnInput(window);
        GameHandler.OnUpdate();
        GameHandler.OnRender();

        // Application Window
        ImGui::SetNextWindowSize(ImVec2(380.f, 110.f));
        ImGui::SetNextWindowPos(ImVec2(10.f, 280.f));

        ImGui::Begin("Application");
        static float acc = 1000.f;

        ImGui::Text("Pack Textures");
        ImGui::SameLine();
        ImGui::InputFloat("Accuracy", &acc);

        if (ImGui::Button("Pack", { 350.f , 20.f })) {
            Packer.PackTextures("res\\images\\block", "res\\images\\sheets\\blocksheet.png", "docs\\texture.yaml", acc);
        }

        ImGui::End();

        ImGuiRender(io);

        // glfw handling
        GLCall(glfwSwapBuffers(window));
        GLCall(glfwPollEvents());
    }

    ImGuiShutdown();
    glfwTerminate();
    return 0;
}