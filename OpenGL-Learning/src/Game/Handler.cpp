#include "Handler.h"

void Handler::OnInit()
{
	// TexturePacker::PackTextures("res/images/block/");

	// Bind Cubemap to static blocks
	m_Skybox.Bind(1, *(m_World.getShaderPackage().shaderBlockStatic));
}

void Handler::DebugWindow()
{
	ImGui::SetNextWindowSize(ImVec2(380.f, 260.f));
	ImGui::SetNextWindowPos(ImVec2(10.f, 10.f));

	unsigned int drawCalls = 0;
	drawCalls += m_World.getDrawCalls();
	drawCalls += m_Skybox.getDrawCalls();

	ImGui::Begin("Debug");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	const glm::vec3& position = m_World.getCameraPosition();
	ImGui::Text(("Position: " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z)).c_str());
	ImGui::Separator();

	ImGui::Text(("Count [Block_Static]: " + std::to_string(m_World.getAmountBlockStatic())).c_str());
	ImGui::Text(("Count [Chunk]: " + std::to_string(m_World.getAmountChunk())).c_str());
	ImGui::Text(("Avail Blocks: " + std::to_string(m_World.getAmountBlockFormat())).c_str());
	ImGui::Text(("Avail Textures: " + std::to_string(m_World.getAmountTextureFormat())).c_str());

	ImGui::Separator();
	ImGui::Text(("Draw Calls: " + std::to_string(drawCalls)).c_str());
	size_t verts = m_World.getDrawnVertices();
	ImGui::Text(("Vertices: " + std::to_string(verts) + "  (avrg. " + std::to_string(((((float)verts / (conf.c_RENDER_DISTANCE * conf.c_RENDER_DISTANCE * 4)) / (conf.c_MAX_BUFFER_FACES * 4)) * 100.f)) + "%% of VBO used)").c_str());

	ImGui::Separator();
	ImGui::Text(("Vertex Buffer Face Size: " + std::to_string(conf.c_MAX_BUFFER_FACES)).c_str());
	ImGui::Text(("Chunk Size: " + std::to_string(conf.c_CHUNK_SIZE)).c_str());
	ImGui::Text(("Terrain Y-Stretch: " + std::to_string(conf.c_TERRAIN_STRETCH_Y)).c_str());
	ImGui::Text(("Render Distance: " + std::to_string(conf.c_RENDER_DISTANCE)).c_str());

	// Keybindings
	ImGui::SetNextWindowSize(ImVec2(380.f, 110.f));
	ImGui::SetNextWindowPos(ImVec2(10.f, 280.f));
	ImGui::Begin("Keybindings");

	ImGui::Text("X:    Toggle Wireframe");
	ImGui::Text("P:    Pack Textures");

	ImGui::End();

	static bool toggleWireframe = false;

	static bool keyPressedX = false;
	if (glfwGetKey(r_Window, GLFW_KEY_X) == GLFW_PRESS && !keyPressedX)
	{
		keyPressedX = true;
		if (toggleWireframe) {
			GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			toggleWireframe = false;
		}
		else {
			GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
			toggleWireframe = true;
		}
	}
	else if (glfwGetKey(r_Window, GLFW_KEY_X) == GLFW_RELEASE && keyPressedX) keyPressedX = false;

	static bool keyPressedP = false;
	if (glfwGetKey(r_Window, GLFW_KEY_P) == GLFW_PRESS && !keyPressedP)
	{
		keyPressedP = true;
		std::cout << "Packing Textures..." << '\n';
		Packer.PackTextures("res\\images\\block", "res\\images\\sheets\\blocksheet.png", "docs\\texture.yaml", conf.c_TEXTURE_INVERSE_OFFSET);
	}
	else if (glfwGetKey(r_Window, GLFW_KEY_X) == GLFW_RELEASE && keyPressedP) keyPressedP = false;

	ImGui::End();
}

Handler::Handler(GLFWwindow* window)
	:r_Window(window), m_World(window), m_Skybox("res/images/skybox/sky5/sky", ".jpg")
{
	// GLFW Input Mode configuration
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// GL Flags
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glEnable(GL_DEPTH_TEST));

	// Enabling Face Culling
	GLCall(glEnable(GL_CULL_FACE));
	GLCall(glCullFace(GL_BACK));
	GLCall(glFrontFace(GL_CCW));

	// Anti Aliasing
	GLCall(glEnable(GL_MULTISAMPLE));

	OnInit();
}

void Handler::OnInput(GLFWwindow* window)
{
	m_World.OnInput(window, v_DeltaTime);
}

void Handler::OnRender()
{
	m_World.OnRender();

	// Draw Skybox at end
	m_Skybox.OnRender();
}

void Handler::OnUpdate()
{
	m_World.OnUpdate(v_DeltaTime);
	m_Skybox.setMatrix(m_World.getMatrixProjection(), m_World.getMatrixView());

	float currentFrame = (float)glfwGetTime();
	v_DeltaTime = currentFrame - v_LastFrame;
	v_LastFrame = currentFrame;

#ifdef _DEBUG
	DebugWindow();
#endif // !_DEBUG
}
