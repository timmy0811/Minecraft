#include "World.h"

World::World(GLFWwindow* window)
	:m_ShaderPackage{ new Shader("res/shaders/block/static/shader_static.vert", "res/shaders/block/static/shader_static.frag") },
	r_Window(window),
	m_TextureMap("res/images/sheets/blocksheet.png", false),
	m_Noise(siv::PerlinNoise::seed_type(std::time(NULL))),
	m_GenerationSemaphore(0)
{
	m_MatrixView = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -3.5f));
	m_MatrixProjection = glm::perspective(glm::radians(45.f), (float)conf.WIN_WIDTH / (float)conf.WIN_HEIGHT, 0.1f, 100.f);

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformMat4f("u_Projection", m_MatrixProjection);

	// Experimental Texture Setup
	m_TextureMap.Bind(0);

	m_ShaderPackage.shaderBlockStatic->SetUniform1i("u_TextureMap", m_TextureMap.GetBoundPort());

	// Parse blocks before textures!!!
	ParseBlocks("docs/block.yaml");
	ParseTextures("docs/texture.yaml");

	for (int i = 0; i < conf.GENERATION_THREADS; i++) {
		m_GenerationThreads.push_back(std::thread([this]() {
			this->GenerationThreadJob();
			}));
	}
	
	m_Chunks.reserve(conf.RENDER_DISTANCE * conf.RENDER_DISTANCE * 4);
	GenerateTerrain();
	SetupLight();
}

World::~World()
{
	for (Chunk* chunk : m_Chunks) {
		delete chunk;
	}
	m_Chunks.clear();

	// Safely exiting worker threads
	m_ExecuteGenerationJob = false;
	m_GenerationSemaphore.release(1000);
	for (auto& thread : m_GenerationThreads) {
		if(thread.joinable()) thread.join();
	}
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
	HandleChunkLoading();

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

const glm::vec3& World::getCameraPosition() const
{
	return m_Camera.Position;
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
	glm::vec3 chunkRootPosition = {- (int)(conf.RENDER_DISTANCE * conf.CHUNK_SIZE * conf.BLOCK_SIZE), 0.f, - (int)(conf.RENDER_DISTANCE * conf.CHUNK_SIZE * conf.BLOCK_SIZE) };

	// Instantiation Phase
	glm::vec3 chunkOffset = { 0.f, 0.f, 0.f };	
	for (int i = 0; i < conf.RENDER_DISTANCE * conf.RENDER_DISTANCE * 4; i++) {
		m_Chunks[i] = new Chunk(&m_BlockFormats, &m_TextureFormats);
		m_Chunks[i]->setID(i);
	}

	// Neighboring Phase
	for (int i = 0; i < m_Chunks.size(); i++) {
		glm::vec2 coord = IndexToCoord(i);

		Chunk* c1 = coord.y > 0									? m_Chunks[CoordToIndex({ coord.x + 0, coord.y - 1 })] : nullptr;
		Chunk* c2 = coord.x < conf.RENDER_DISTANCE * 2 - 1		? m_Chunks[CoordToIndex({ coord.x + 1, coord.y + 0 })] : nullptr;
		Chunk* c3 = coord.y < conf.RENDER_DISTANCE * 2 -1		? m_Chunks[CoordToIndex({ coord.x + 0, coord.y + 1 })] : nullptr;
		Chunk* c4 = coord.x > 0									? m_Chunks[CoordToIndex({ coord.x - 1, coord.y + 0 })] : nullptr;

		m_Chunks[i]->setChunkNeighbors(c1, c2, c3, c4);
	}

	// Terrain Generation Phase
	size_t offset = 0;
	for (int chunkX = 0; chunkX < 2 * conf.RENDER_DISTANCE; chunkX++) {
		chunkOffset.z = 0.f;
		for (int chunkZ = 0; chunkZ < 2 * conf.RENDER_DISTANCE; chunkZ++) {
			m_Chunks[offset]->setGenerationData(chunkRootPosition + chunkOffset, { chunkX * 1.f, chunkZ * 1.f, 1.f }, m_Noise);

			if (conf.ENABLE_MULTITHREADING) {
				m_ChunksQueuedGenerating.push(m_Chunks[offset]);
				m_GenerationSemaphore.release(1);
			}
			else {
				m_Chunks[offset]->Generate();
			}
			
			chunkOffset.z += conf.BLOCK_SIZE * conf.CHUNK_SIZE;
			offset++;
		}
		chunkOffset.x += conf.BLOCK_SIZE * conf.CHUNK_SIZE;
	}
}

inline unsigned int World::CoordToIndex(const glm::vec2& coord) const
{
	return (unsigned int)(coord.x * (float)conf.RENDER_DISTANCE * 2.f + coord.y);
}

inline const glm::vec2 World::IndexToCoord(unsigned int index) const
{
	return { std::floor(index / (conf.RENDER_DISTANCE * 2)), index % (conf.RENDER_DISTANCE * 2) };
}

void World::GenerationThreadJob()
{
	while (true) { 
		m_GenerationSemaphore.acquire();
		if (!m_ExecuteGenerationJob) return;

		// Unload Queued Chunks
		if (ContainsElementAtomic(&m_ChunksQueuedUnloading, m_MutexLoading)) {
			m_MutexLoading.lock();
			Chunk* chunk = m_ChunksQueuedUnloading.front();
			//chunk->Unload();
			m_ChunksQueuedUnloading.pop();
			m_MutexLoading.unlock();
		}
		// Load Queued Chunks
		else if (ContainsElementAtomic(&m_ChunksQueuedLoading, m_MutexLoading)) {
			m_MutexLoading.lock();
			Chunk* chunk = m_ChunksQueuedLoading.front();
			//chunk->Load();
			m_ChunksQueuedLoading.pop();
			m_MutexLoading.unlock();
		}
		// Generate and Cull Queued Chunks
		else if (ContainsElementAtomic(&m_ChunksQueuedGenerating, m_MutexGenerating)) {
			m_IsGenerating++;
			m_MutexGenerating.lock();
			Chunk* chunk = m_ChunksQueuedGenerating.front();
			m_ChunksQueuedGenerating.pop();
			m_MutexGenerating.unlock();

			LOG(("Generating Chunk " + std::to_string(chunk->getID())));
			chunk->Generate();
			LOG(("Done Generating Chunk " + std::to_string(chunk->getID())));

			m_MutexCullFaces.lock();
			m_ChunksQueuedCulling.push(chunk);
			m_MutexCullFaces.unlock();

			m_IsGenerating--;
			m_GenerationSemaphore.release();
		}
		// Cull Faces of Queued Chunks
		else if (ContainsElementAtomic(&m_ChunksQueuedCulling, m_MutexCullFaces) && !ContainsElementAtomic(&m_ChunksQueuedGenerating, m_MutexGenerating)) {
			if (m_IsGenerating == 0) {
				LOG(("Entering Culling"));
				m_MutexCullFaces.lock();
				Chunk* chunk = m_ChunksQueuedCulling.front();
				m_ChunksQueuedCulling.pop();
				m_MutexCullFaces.unlock();

				LOG(("Culling Chunk " + std::to_string(chunk->getID())));
				chunk->CullFacesOnLoadBuffer();
				LOG(("Done Culling Chunk " + std::to_string(chunk->getID())));

				m_MutexBufferLoading.lock();
				m_ChunksQueuedBufferLoading.push(chunk);
				m_MutexBufferLoading.unlock();
			}
			else {
				m_GenerationSemaphore.release();
			}
		}
		else {
			LOG(("Doing Nothing"));
		}
	}
}

void World::HandleChunkLoading()
{
	// Buffering Phase
	m_MutexBufferLoading.lock();
	for (int i = 0; i < m_ChunksQueuedBufferLoading.size(); i++) {
		Chunk* ch = m_ChunksQueuedBufferLoading.front();
		LOG(("Loading Chunk " + std::to_string(ch->getID())));
		ch->LoadVertexBufferFromLoadBuffer();
		m_ChunksQueuedBufferLoading.pop();
	}
	m_MutexBufferLoading.unlock();

	//if (chunkUnload) {
	//	// ...
	//	m_ChunksQueuedLoading.push(Chunk);
	//	m_GenerationThreadActions++;
	//	m_ConditionGeneration.notify_one();
	//}
}

bool World::ContainsElementAtomic(std::queue<Chunk*>* list, std::mutex& mutex)
{
	bool containsElement = false;
	mutex.lock();
	if (list->size() > 0) containsElement = true;
	mutex.unlock();
	return containsElement;
}

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

		m_UsedTextures.insert(blockFormat.texture_top);
		m_UsedTextures.insert(blockFormat.texture_bottom);
		m_UsedTextures.insert(blockFormat.texture_left);

		m_UsedTextures.insert(blockFormat.texture_right);
		m_UsedTextures.insert(blockFormat.texture_back);
		m_UsedTextures.insert(blockFormat.texture_front);

		m_BlockFormats[blockFormat.id] = blockFormat;
	}
}

void World::ParseTextures(const std::string& path)
{
	YAML::Node mainNode = YAML::LoadFile(path);
	for (auto texture : mainNode) {
		Minecraft::Texture_Format textureFormat;

		textureFormat.name = texture.first.as<std::string>();
		if (m_UsedTextures.find(textureFormat.name) == m_UsedTextures.end()) continue;

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
	m_DirLight.ambient = { 0.65f, 0.65f, 0.65f };
	m_DirLight.diffuse = { 1.0f, 1.0f, 1.0f };
	m_DirLight.specular = { 0.2f, 0.20f, 0.20f };
	m_DirLight.direction = { 1.f, -1.f, 0.5f };

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformDirectionalLight("u_DirLight", m_DirLight);

	// Fog
	m_ShaderPackage.shaderBlockStatic->SetUniform1f("u_FogAffectDistance", conf.FOG_AFFECT_DISTANCE);
	m_ShaderPackage.shaderBlockStatic->SetUniform1f("u_FogDensity", conf.FOG_DENSITY);
	m_ShaderPackage.shaderBlockStatic->SetUniform3f("u_SkyBoxColor", 0.7568627f, 0.850980f, 0.858823f);
}

void World::updateLight()
{

}

void World::OnMouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	World::s_MouseX = float(xpos);
	World::s_MouseY = float(ypos);
}

