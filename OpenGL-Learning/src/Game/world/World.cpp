#include "World.h"

World::World(GLFWwindow* window)
	:m_ShaderPackage{ new Shader("res/shaders/block/static/shader_static.vert", "res/shaders/block/static/shader_static.frag") },
	r_Window(window),
	m_TextureMap("res/images/sheets/blocksheet.png", false),
	m_Noise(siv::PerlinNoise::seed_type(std::time(NULL))),
	m_GenerationSemaphore(0)
{
	m_MatrixView = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -3.5f));
	m_MatrixProjection = glm::perspective(glm::radians(45.f), (float)conf.WIN_WIDTH / (float)conf.WIN_HEIGHT, 0.1f, 300.f);

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformMat4f("u_Projection", m_MatrixProjection);

	// Experimental Texture Setup
	m_TextureMap.Bind(0);

	m_ShaderPackage.shaderBlockStatic->SetUniform1i("u_TextureMap", m_TextureMap.GetBoundPort());

	// Parse blocks before textures!!!
	ParseBlocks("docs/block.yaml");
	ParseTextures("docs/texture.yaml");

	for (unsigned int i = 0; i < conf.GENERATION_THREADS; i++) {
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
		if (!chunk || !chunk->isLoaded()) continue;
		chunk->OnRender(m_ShaderPackage, m_Camera.Position);
	}

	// Render transparent objects
	for (Chunk* chunk : m_Chunks) {
		if (!chunk) continue;
		chunk->OnRenderTransparents(m_ShaderPackage, m_Camera.Position);
	}
}

void World::NeighborChunks()
{
	for (unsigned int x = 0; x < conf.WORLD_WIDTH; x++) {
		for (unsigned int z = 0; z < conf.WORLD_WIDTH; z++) {
			Chunk* chunk = m_Chunks[CoordToIndex({ x, z })];
			if (!chunk) continue;

			chunk->setChunkNeighbors(
				z > 0						? m_Chunks[CoordToIndex({ x - 0, z - 1 })] : nullptr,
				x < conf.WORLD_WIDTH - 1	? m_Chunks[CoordToIndex({ x + 1, z - 0 })] : nullptr,
				z < conf.WORLD_WIDTH - 1	? m_Chunks[CoordToIndex({ x - 0, z + 1 })] : nullptr,
				x > 0						? m_Chunks[CoordToIndex({ x - 1, z - 0 })] : nullptr
			);
		}
	}
}

void World::OnUpdate(double deltaTime)
{
	glfwSetCursorPosCallback(r_Window, OnMouseCallback);

	ProcessMouse();
	m_MatrixView = glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Front, m_Camera.Up);

	UpdateLight();
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
		if (!chunk) continue;
		amount += (unsigned int)chunk->getAmountBlockStatic();
	}

	return amount;
}

const size_t World::getDrawnVertices() const
{
	size_t verts = 0;
	for (Chunk* chunk : m_Chunks) {
		if (!chunk) continue;
		verts += chunk->getDrawnVertices();
	}

	return verts;
}

const unsigned int World::getDrawCalls() const
{
	unsigned int calls = 0;
	for (Chunk* chunk : m_Chunks) {
		if (!chunk) continue;
		calls += chunk->getDrawCalls();
	}

	return calls;
}

void World::OnInput(GLFWwindow* window, double deltaTime)
{
	const float cameraSpeed = 20.f * (float)deltaTime; // adjust accordingly

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
	if (m_FirstMouseInit) {
		m_LastX = s_MouseX;
		m_LastY = s_MouseY;
		m_FirstMouseInit = false;
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
	unsigned int chunkWidth = (unsigned int)(conf.CHUNK_SIZE * conf.BLOCK_SIZE);

	glm::vec2 rootPosition = { conf.WORLD_WIDTH / 2, conf.WORLD_WIDTH / 2 };															// Spawn Chunk in Matric Space
	glm::vec2 generationPosition = { conf.WORLD_WIDTH / 2 - conf.RENDER_DISTANCE, conf.WORLD_WIDTH / 2  - conf.RENDER_DISTANCE};		// Upper left edge of Spawn area in Matrix Space
	m_WorldRootPosition = { -(int)((rootPosition.x + 0.5) * chunkWidth), 0, -(int)((rootPosition.y + 0.5) * chunkWidth) };				// Upper left edge of World in World Space

	glm::vec3 chunkOffset = { 0.f, 0.f, 0.f };	
	for (unsigned int x = 0; x < conf.RENDER_DISTANCE * 2 + 1; x++) {
		chunkOffset.z = 0.f;
		for (unsigned int z = 0; z < conf.RENDER_DISTANCE * 2 + 1; z++) {
			const unsigned int i = CoordToIndex(generationPosition + glm::vec2{ x, z });
			m_Chunks[i] = new Chunk(&m_BlockFormats, &m_TextureFormats);
			m_Chunks[i]->setID(i);
			m_Chunks[i]->setGenerationData({ (-(int)((conf.RENDER_DISTANCE + 0.5) * chunkWidth)) + chunkOffset.x, 0, (-(int)((conf.RENDER_DISTANCE + 0.5) * chunkWidth)) + chunkOffset.z },
				{ (generationPosition.x + x) * 1.f, (generationPosition.y + z) * 1.f, 1.f }, m_Noise);

			if (conf.ENABLE_MULTITHREADING) m_ChunksQueuedGenerating.push_back(m_Chunks[i]);
			else m_Chunks[i]->Generate();

			chunkOffset.z += conf.BLOCK_SIZE * conf.CHUNK_SIZE;
		}
		chunkOffset.x += conf.BLOCK_SIZE * conf.CHUNK_SIZE;
	}

	// Neighboring
	for (int i = 0; i < m_Chunks.size(); i++) {
		if (!m_Chunks[i]) continue;
		glm::vec2 coord = IndexToCoord(i);

		Chunk* c1 = coord.y > 0									? m_Chunks[CoordToIndex({ coord.x + 0, coord.y - 1 })] : nullptr;
		Chunk* c2 = coord.x < conf.WORLD_WIDTH - 1				? m_Chunks[CoordToIndex({ coord.x + 1, coord.y + 0 })] : nullptr;
		Chunk* c3 = coord.y < conf.WORLD_WIDTH - 1				? m_Chunks[CoordToIndex({ coord.x + 0, coord.y + 1 })] : nullptr;
		Chunk* c4 = coord.x > 0									? m_Chunks[CoordToIndex({ coord.x - 1, coord.y + 0 })] : nullptr;

		m_Chunks[i]->setChunkNeighbors(c1, c2, c3, c4);
	}

	if(conf.ENABLE_MULTITHREADING) m_GenerationSemaphore.release(m_ChunksQueuedGenerating.size());
}

inline int World::CoordToIndex(const glm::vec2& coord) const
{
	if (coord.x < 0 || coord.x >= conf.WORLD_WIDTH || coord.y < 0 || coord.y >= conf.WORLD_WIDTH) return -1;
	return (unsigned int)(coord.x * (float)conf.WORLD_WIDTH + coord.y);
}

inline const glm::vec2 World::IndexToCoord(unsigned int index) const
{
	return { std::floor(index / conf.WORLD_WIDTH), index % (conf.WORLD_WIDTH) };
}

void World::GenerationThreadJob()
{
	while (true) { 
		m_GenerationSemaphore.acquire();
		if (!m_ExecuteGenerationJob) return;

		// Unload Queued Chunks
		if (ContainsElementAtomic(&m_ChunksQueuedDeserialize, m_MutexLoading)) {
			m_MutexLoading.lock();
			Chunk* chunk = m_ChunksQueuedDeserialize.front();
			chunk->Deserialize();
			m_ChunksQueuedDeserialize.pop_front();
			m_MutexLoading.unlock();
			continue;
		}
		// Load Queued Chunks
		if (ContainsElementAtomic(&m_ChunksQueuedSerialize, m_MutexLoading)) {
			m_MutexLoading.lock();
			Chunk* chunk = m_ChunksQueuedSerialize.front();
			chunk->Serialize();
			m_ChunksQueuedSerialize.pop_front();
			m_MutexLoading.unlock();
			continue;
		}
		// Generate and Cull Queued Chunks
		if (ContainsElementAtomic(&m_ChunksQueuedGenerating, m_MutexGenerating)) {
			m_IsGenerating++;
			m_MutexGenerating.lock();
			if (!m_ChunksQueuedGenerating.empty()) {
				Chunk* chunk = m_ChunksQueuedGenerating.front();
				m_ChunksQueuedGenerating.pop_front();
				m_MutexGenerating.unlock();

				chunk->Generate();

				m_MutexCullFaces.lock();
				m_ChunksQueuedCulling.push_back(chunk);
				m_MutexCullFaces.unlock();

				m_IsGenerating--;
				m_GenerationSemaphore.release();
				continue;
			}
			m_MutexGenerating.unlock();
		}
		// Cull Faces of Queued Chunks
		if (ContainsElementAtomic(&m_ChunksQueuedCulling, m_MutexCullFaces) && !ContainsElementAtomic(&m_ChunksQueuedGenerating, m_MutexGenerating)) {
			if (m_IsGenerating == 0) {
				m_MutexCullFaces.lock();
				Chunk* chunk = m_ChunksQueuedCulling.front();
				m_ChunksQueuedCulling.pop_front();
				m_MutexCullFaces.unlock();
				chunk->CullFacesOnLoadBuffer();

				m_MutexBufferLoading.lock();
				m_ChunksQueuedBufferLoading.push_back(chunk);
				m_MutexBufferLoading.unlock();
			}
			else {
				m_GenerationSemaphore.release();
			}
			continue;
		}
		LOG("Warning: Threading skipped a task!");
	}
}

void World::HandleChunkLoading()
{
	unsigned int waitingForNeighboring = 0;
	bool hasExpanded = false;
	std::unordered_set<Chunk*> ChunksReculled;

	// Buffering Phase
	m_MutexBufferLoading.lock();
	for (int i = 0; i < m_ChunksQueuedBufferLoading.size(); i++) {
		Chunk* chunk = m_ChunksQueuedBufferLoading.front();
		// LOG(("Loading Chunk " + std::to_string(chunk->getID())));
		chunk->LoadVertexBufferFromLoadBuffer();
		m_ChunksQueuedBufferLoading.pop_front();
	}
	m_MutexBufferLoading.unlock();
	
	glm::vec2 currentPLayerChunkPosition = { std::floor(abs(m_WorldRootPosition.x - m_Camera.Position.x) / conf.CHUNK_SIZE), std::floor(abs(m_WorldRootPosition.z - m_Camera.Position.z) / conf.CHUNK_SIZE) };

	if (m_IsGenerationInit) {
		m_PlayerChunkPosition = currentPLayerChunkPosition;
		m_IsGenerationInit = false;
	}

	if (conf.EXPAND_TERRAIN && currentPLayerChunkPosition.x < m_PlayerChunkPosition.x) {
		hasExpanded = true;
		// Expand in X-
		int startX = (int)(m_PlayerChunkPosition.x - conf.RENDER_DISTANCE - 1);
		int startZ = (int)(m_PlayerChunkPosition.y - conf.RENDER_DISTANCE - 0);

		for (unsigned int i = 0; i < conf.RENDER_DISTANCE * 2 + 1; i++) {
			unsigned int index = CoordToIndex({ startX, startZ + i });
			if (index == -1) continue;
			if (m_Chunks[index]) {
				m_Chunks[index]->LoadVertexBufferFromLoadBuffer();
			}
			else {
				Chunk* chunk = new Chunk(&m_BlockFormats, &m_TextureFormats);
				m_Chunks[index] = chunk;
				chunk->setID(index);
				chunk->setGenerationData({ m_WorldRootPosition.x + startX * conf.CHUNK_SIZE, 0, m_WorldRootPosition.z + (startZ + i) * conf.CHUNK_SIZE },
					{ (startX) * 1.f, (startZ + i) * 1.f, 1.f }, m_Noise);

				Chunk* chunkNeighbor = m_Chunks[CoordToIndex({ startX + 1, startZ + i })];
				if (chunkNeighbor) ChunksReculled.insert(chunkNeighbor);

				chunkNeighbor = startX > 0 ? m_Chunks[CoordToIndex({ startX - 1, startZ + i })] : nullptr;
				if (chunkNeighbor) ChunksReculled.insert(chunkNeighbor);

				m_MutexGenerating.lock();
				m_ChunksQueuedGenerating.push_back(chunk);
				waitingForNeighboring++;
				m_MutexGenerating.unlock();
			}

			// Unload in X+
			unsigned int unloadIndex = CoordToIndex({ m_PlayerChunkPosition.x + conf.RENDER_DISTANCE, startZ + i });
			Chunk* chunkUnload = unloadIndex != -1 ? m_Chunks[unloadIndex] : nullptr;
			if (chunkUnload && chunkUnload->isLoaded()) chunkUnload->Unload();
		}
	}
	else if (conf.EXPAND_TERRAIN && currentPLayerChunkPosition.x > m_PlayerChunkPosition.x) {
		hasExpanded = true;
		// Expand in X+
		int startX = (int)(m_PlayerChunkPosition.x + conf.RENDER_DISTANCE + 1);
		int startZ = (int)(m_PlayerChunkPosition.y - conf.RENDER_DISTANCE - 0);

		for (unsigned int i = 0; i < conf.RENDER_DISTANCE * 2 + 1; i++) {
			unsigned int index = CoordToIndex({ startX, startZ + i });
			if (index == -1) continue;
			if (m_Chunks[index]) {
				m_Chunks[index]->LoadVertexBufferFromLoadBuffer();
			}
			else {
				Chunk* chunk = new Chunk(&m_BlockFormats, &m_TextureFormats);
				m_Chunks[index] = chunk;
				chunk->setID(index);
				chunk->setGenerationData({ m_WorldRootPosition.x + startX * conf.CHUNK_SIZE, 0, m_WorldRootPosition.z + (startZ + i) * conf.CHUNK_SIZE },
					{ (startX) * 1.f, (startZ + i) * 1.f, 1.f }, m_Noise);

				Chunk* chunkNeighbor = m_Chunks[CoordToIndex({ startX - 1, startZ + i })];
				if (chunkNeighbor) ChunksReculled.insert(chunkNeighbor);

				chunkNeighbor = (unsigned int)startX < conf.WORLD_WIDTH - 1 ? m_Chunks[CoordToIndex({ startX + 1, startZ + i })] : nullptr;
				if (chunkNeighbor) ChunksReculled.insert(chunkNeighbor);

				m_MutexGenerating.lock();
				m_ChunksQueuedGenerating.push_back(chunk);
				waitingForNeighboring++;
				m_MutexGenerating.unlock();
			}

			// Unload in X-
			unsigned int unloadIndex = CoordToIndex({ m_PlayerChunkPosition.x - conf.RENDER_DISTANCE, startZ + i });
			Chunk* chunkUnload = unloadIndex != -1 ? m_Chunks[unloadIndex] : nullptr;
			if (chunkUnload && chunkUnload->isLoaded()) chunkUnload->Unload();
		}
	}

	if (conf.EXPAND_TERRAIN && currentPLayerChunkPosition.y < m_PlayerChunkPosition.y) {
		hasExpanded = true;
		// Expand in Z-
		int startX = (int)(m_PlayerChunkPosition.x - conf.RENDER_DISTANCE - 0);
		int startZ = (int)(m_PlayerChunkPosition.y - conf.RENDER_DISTANCE - 1);

		for (unsigned int i = 0; i < conf.RENDER_DISTANCE * 2 + 1; i++) {
			unsigned int index = CoordToIndex({ startX + i, startZ });
			if (index == -1) continue;
			if (m_Chunks[index]) {
				m_Chunks[index]->LoadVertexBufferFromLoadBuffer();
			}
			else {
				Chunk* chunk = new Chunk(&m_BlockFormats, &m_TextureFormats);
				m_Chunks[index] = chunk;
				chunk->setID(index);
				chunk->setGenerationData({ m_WorldRootPosition.x + (startX + i) * conf.CHUNK_SIZE, 0, m_WorldRootPosition.z + startZ * conf.CHUNK_SIZE },
					{ (startX + i) * 1.f, (startZ + 0) * 1.f, 1.f }, m_Noise);

				Chunk* chunkNeighbor = m_Chunks[CoordToIndex({ startX + i, startZ + 1 })];
				if (chunkNeighbor) ChunksReculled.insert(chunkNeighbor);

				chunkNeighbor = startX > 0 ? m_Chunks[CoordToIndex({ startX + i, startZ - 1 })] : nullptr;
				if (chunkNeighbor) ChunksReculled.insert(chunkNeighbor);

				m_MutexGenerating.lock();
				m_ChunksQueuedGenerating.push_back(chunk);
				waitingForNeighboring++;
				m_MutexGenerating.unlock();
			}

			// Unload in Z+
			unsigned int unloadIndex = CoordToIndex({ startX + i, m_PlayerChunkPosition.y + conf.RENDER_DISTANCE });
			Chunk* chunkUnload = unloadIndex != -1 ? m_Chunks[unloadIndex] : nullptr;
			if (chunkUnload && chunkUnload->isLoaded()) chunkUnload->Unload();
		}
	}
	else if (conf.EXPAND_TERRAIN && currentPLayerChunkPosition.y > m_PlayerChunkPosition.y) {
		hasExpanded = true;
		// Expand in Z+
		int startX = (int)(m_PlayerChunkPosition.x - conf.RENDER_DISTANCE - 0);
		int startZ = (int)(m_PlayerChunkPosition.y + conf.RENDER_DISTANCE + 1);

		for (unsigned int i = 0; i < conf.RENDER_DISTANCE * 2 + 1; i++) {
			unsigned int index = CoordToIndex({ startX + i, startZ });
			if (index == -1) continue;
			if (m_Chunks[index]) {
				m_Chunks[index]->LoadVertexBufferFromLoadBuffer();
			}
			else {
				Chunk* chunk = new Chunk(&m_BlockFormats, &m_TextureFormats);
				m_Chunks[index] = chunk;
				chunk->setID(index);
				chunk->setGenerationData({ m_WorldRootPosition.x + (startX + i) * conf.CHUNK_SIZE, 0, m_WorldRootPosition.z + startZ * conf.CHUNK_SIZE },
					{ (startX + i) * 1.f, (startZ + 0) * 1.f, 1.f }, m_Noise);

				Chunk* chunkNeighbor = m_Chunks[CoordToIndex({ startX + i, startZ - 1 })];
				if (chunkNeighbor) ChunksReculled.insert(chunkNeighbor);

				chunkNeighbor = (unsigned int)startZ < conf.WORLD_WIDTH - 1 ? m_Chunks[CoordToIndex({ startX + i, startZ + 1 })] : nullptr;
				if (chunkNeighbor) ChunksReculled.insert(chunkNeighbor);

				m_MutexGenerating.lock();
				m_ChunksQueuedGenerating.push_back(chunk);
				waitingForNeighboring++;
				m_MutexGenerating.unlock();
			}
			// Unload in Z-
			unsigned int unloadIndex = CoordToIndex({ startX + i, m_PlayerChunkPosition.y - conf.RENDER_DISTANCE });
			Chunk* chunkUnload = unloadIndex != -1 ? m_Chunks[unloadIndex] : nullptr;
			if (chunkUnload && chunkUnload->isLoaded()) chunkUnload->Unload();
		}
	}

	if (hasExpanded) NeighborChunks();

	for (auto it = ChunksReculled.begin(); it != ChunksReculled.end(); it++) {
		m_ChunksQueuedCulling.push_back(*it);
	}

	m_GenerationSemaphore.release(waitingForNeighboring + ChunksReculled.size());
	m_PlayerChunkPosition = currentPLayerChunkPosition;
}

bool World::ContainsElementAtomic(std::deque<Chunk*>* list, std::mutex& mutex)
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
	m_ShaderPackage.shaderBlockStatic->SetUniform3f("u_SkyBoxColor", conf.FOG_COLOR.r / 255.f, conf.FOG_COLOR.g / 255.f, conf.FOG_COLOR.b / 255.f);
}

void World::UpdateLight()
{

}

void World::OnMouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	World::s_MouseX = float(xpos);
	World::s_MouseY = float(ypos);
}

