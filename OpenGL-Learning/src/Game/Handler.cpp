#include "Handler.h"

void Handler::OnInit()
{
	// TexturePacker::PackTextures("res/images/block/");

	// Bind Cubemap to static blocks
	m_Skybox.Bind(1, *(m_World.getShaderPackage().shaderBlockStatic));
}

void Handler::DebugWindow()
{
	ImGui::SetNextWindowSize(ImVec2(380.f, 240.f));
	ImGui::SetNextWindowPos(ImVec2(10.f, 10.f));

	unsigned int drawCalls = 0;
	drawCalls += m_World.getDrawCalls();
	drawCalls += m_Skybox.getDrawCalls();

	ImGui::Begin("Debug");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Separator();

	ImGui::Text(("Count [Block_Static]: " + std::to_string(m_World.getAmountBlockStatic())).c_str());
	ImGui::Text(("Count [Chunk]: " + std::to_string(m_World.getAmountChunk())).c_str());
	ImGui::Text(("Avail Blocks: " + std::to_string(m_World.getAmountBlockFormat())).c_str());
	ImGui::Text(("Avail Textures: " + std::to_string(m_World.getAmountTextureFormat())).c_str());

	ImGui::Separator();
	ImGui::Text(("Draw Calls: " + std::to_string(drawCalls)).c_str());
	size_t verts = m_World.getDrawnVertices();
	ImGui::Text(("Vertices: " + std::to_string(verts) + "     (" + std::to_string(((((float)verts / (c_RenderDistanceStatic * c_RenderDistanceStatic)) / (c_BatchFaceCount * 4)) * 100.f)) + "%% of VBO used)").c_str());

	ImGui::Separator();
	ImGui::Text(("Vertex Buffer Face Size: " + std::to_string(c_BatchFaceCount)).c_str());
	ImGui::Text(("Chunk Size: " + std::to_string(c_ChunkSize)).c_str());
	ImGui::Text(("Terrain Y-Stretch: " + std::to_string(c_TerrainYStretch)).c_str());
	ImGui::Text(("Render Distance: " + std::to_string(c_RenderDistanceStatic)).c_str());

	ImGui::End();
}

Handler::Handler(GLFWwindow* window)
	:r_Window(window), m_World(window), m_Skybox("res/images/skybox/sky3/sky", ".jpg")
{
	// GLFW Input Mode configuration
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GL Flags
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glEnable(GL_DEPTH_TEST));

	// Enabling Face Culling
	GLCall(glEnable(GL_CULL_FACE));
	GLCall(glCullFace(GL_BACK));
	GLCall(glFrontFace(GL_CCW));

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
	m_World.OnUpdate();
	m_Skybox.setMatrix(m_World.getMatrixProjection(), m_World.getMatrixView());

	float currentFrame = (float)glfwGetTime();
	v_DeltaTime = currentFrame - v_LastFrame;
	v_LastFrame = currentFrame;

#ifdef _DEBUG
	DebugWindow();
#endif // !_DEBUG
}
