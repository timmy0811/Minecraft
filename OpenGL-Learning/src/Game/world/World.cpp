#include "World.h"

World::World(GLFWwindow* window)
	:m_ShaderPackage{ new Shader("res/shaders/block/static/shader_static.vert", "res/shaders/block/static/shader_static.frag") },
	r_Window(window),
	m_TextureMap("res/images/sheets/blocksheet.png", false),
	m_Noise(siv::PerlinNoise::seed_type(std::time(NULL)))
{
	m_MatrixView = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -3.5f));
	m_MatrixProjection = glm::perspective(glm::radians(45.f), (float)c_win_Width / (float)c_win_Height, 0.1f, 100.f);

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformMat4f("u_Projection", m_MatrixProjection);

	// Experimental Texture Setup
	m_TextureMap.Bind(0);

	m_ShaderPackage.shaderBlockStatic->SetUniform1i("u_TextureMap", m_TextureMap.GetBoundPort());

	ParseBlocks("docs/block.yaml");
	ParseTextures("docs/texture.yaml");

	GenerateTerrain();
	SetupLight();
}

World::~World()
{
	for (Chunk* chunk : m_Chunks) {
		delete chunk;
	}
	m_Chunks.clear();
}

void World::OnRender()
{
	// Render opac objects
	for (Chunk* chunk : m_Chunks) {
		chunk->OnRender(m_ShaderPackage, m_Camera.Position);
	}

	// Render transparent objects
	for (Chunk* chunk : m_Chunks) {
		chunk->OnRenderTransparents(m_ShaderPackage, m_Camera.Position);
	}
}

void World::OnUpdate(double deltaTime)
{
	glfwSetCursorPosCallback(r_Window, OnMouseCallback);

	ProcessMouse();
	m_MatrixView = glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Front, m_Camera.Up);

	updateLight();

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformMat4f("u_View", m_MatrixView);
	m_ShaderPackage.shaderBlockStatic->SetUniform3f("u_ViewPosition", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);
}

const glm::mat4& World::getMatrixProjection() const
{
	return m_MatrixProjection;
}

const glm::mat4& World::getMatrixView() const
{
	return m_MatrixView;
}

const unsigned int World::getAmountBlockStatic() const
{
	unsigned int amount = 0;
	for (Chunk* chunk : m_Chunks) {
		amount += (unsigned int)chunk->getAmountBlockStatic();
	}

	return amount;
}

const size_t World::getDrawnVertices() const
{
	size_t verts = 0;
	for (Chunk* chunk : m_Chunks) {
		verts += chunk->getDrawnVertices();
	}

	return verts;
}

const unsigned int World::getDrawCalls() const
{
	unsigned int calls = 0;
	for (Chunk* chunk : m_Chunks) {
		calls += chunk->getDrawCalls();
	}

	return calls;
}

void World::OnInput(GLFWwindow* window, double deltaTime)
{
	const float cameraSpeed = 10.f * (float)deltaTime; // adjust accordingly

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_Camera.Position += cameraSpeed * m_Camera.Front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_Camera.Position -= cameraSpeed * m_Camera.Front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_Camera.Position -= glm::normalize(glm::cross(m_Camera.Front, m_Camera.Up)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_Camera.Position += glm::normalize(glm::cross(m_Camera.Front, m_Camera.Up)) * cameraSpeed;
}

void World::ProcessMouse()
{
	if (m_FirstInit) {
		m_LastX = s_MouseX;
		m_LastY = s_MouseY;
		m_FirstInit = false;
	}

	float xoffset = s_MouseX - m_LastX;
	float yoffset = m_LastY - s_MouseY; // reversed since y-coordinates range from bottom to top
	m_LastX = s_MouseX;
	m_LastY = s_MouseY;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	m_Camera.yaw += xoffset;
	m_Camera.pitch += yoffset;

	if (m_Camera.pitch > 89.0f)
		m_Camera.pitch = 89.0f;
	if (m_Camera.pitch < -89.0f)
		m_Camera.pitch = -89.0f;

	glm::vec3 direction;

	direction.x = cos(glm::radians(m_Camera.yaw)) * cos(glm::radians(m_Camera.pitch));
	direction.y = sin(glm::radians(m_Camera.pitch));
	direction.z = sin(glm::radians(m_Camera.yaw)) * cos(glm::radians(m_Camera.pitch));
	m_Camera.Front = glm::normalize(direction);
}

void World::GenerateTerrain()
{
	glm::vec3 chunkRootPosition = {- c_RenderDistanceStatic * c_ChunkSize * c_BlockSize, 0.f, - c_RenderDistanceStatic * c_ChunkSize * c_BlockSize };

	glm::vec3 chunkOffset = { 0.f, 0.f, 0.f };

	for (int chunkX = 0; chunkX < 2 * c_RenderDistanceStatic; chunkX++) {
		chunkOffset.z = 0.f;
		for (int chunkZ = 0; chunkZ < 2 * c_RenderDistanceStatic; chunkZ++) {
			Chunk* chnk = new Chunk(&m_BlockFormats, &m_TextureFormats);
			std::cout << "Generating Chunk " << std::to_string(chunkX * c_RenderDistanceStatic * 2 + chunkZ) << '\n';
			chnk->Generate(chunkRootPosition + chunkOffset, {chunkX * 1.f, chunkZ * 1.f, 1.f}, m_Noise);
			m_Chunks.push_back(chnk);

			chunkOffset.z += c_BlockSize * c_ChunkSize;
		}
		chunkOffset.x += c_BlockSize * c_ChunkSize;
	}
}

#include <iostream>

void World::ParseBlocks(const std::string& path)
{
	YAML::Node mainNode = YAML::LoadFile(path);
	for (auto block : mainNode) {
		Minecraft::Block_format blockFormat;

		blockFormat.type = static_cast<Minecraft::BLOCKTYPE>(block.second["type"].as<unsigned int>());
		blockFormat.name = block.first.as<std::string>();
		blockFormat.id = block.second["id"].as<int>();
		blockFormat.reflection = block.second["reflection"].as<float>();

		blockFormat.texture_top = block.second["texture_top"].as<std::string>();
		blockFormat.texture_bottom = block.second["texture_bottom"].as<std::string>();
		blockFormat.texture_left = block.second["texture_left"].as<std::string>();
		blockFormat.texture_right = block.second["texture_right"].as<std::string>();
		blockFormat.texture_back = block.second["texture_Back"].as<std::string>();
		blockFormat.texture_front = block.second["texture_front"].as<std::string>();

		m_BlockFormats[blockFormat.id] = blockFormat;
	}
}

void World::ParseTextures(const std::string& path)
{
	YAML::Node mainNode = YAML::LoadFile(path);
	for (auto texture : mainNode) {
		Minecraft::Texture_Format textureFormat;

		textureFormat.name = texture.first.as<std::string>();

		textureFormat.uv[0].x = texture.second["uvs"]["0"][0].as<float>();
		textureFormat.uv[0].y = texture.second["uvs"]["0"][1].as<float>();

		textureFormat.uv[1].x = texture.second["uvs"]["1"][0].as<float>();
		textureFormat.uv[1].y = texture.second["uvs"]["1"][1].as<float>();

		textureFormat.uv[2].x = texture.second["uvs"]["2"][0].as<float>();
		textureFormat.uv[2].y = texture.second["uvs"]["2"][1].as<float>();

		textureFormat.uv[3].x = texture.second["uvs"]["3"][0].as<float>();
		textureFormat.uv[3].y = texture.second["uvs"]["3"][1].as<float>();

		m_TextureFormats[textureFormat.name] = textureFormat;
	}
}

void World::SetupLight()
{
	// Direct Light
	m_DirLight.ambient = { 0.3f, 0.3f, 0.3f };
	m_DirLight.diffuse = { 1.0f, 1.0f, 1.0f };
	m_DirLight.specular = { 0.25f, 0.25f, 0.25f };
	m_DirLight.direction = { 1.f, -1.f, 0.5f };

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformDirectionalLight("u_DirLight", m_DirLight);
}

void World::updateLight()
{

}

void World::OnMouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	World::s_MouseX = float(xpos);
	World::s_MouseY = float(ypos);
}

