#include "World.h"

World::World(GLFWwindow* window)
	:m_TextureMap("res/images/sheets/blocksheet.png", false),
	m_CountChunks(conf.WORLD_WIDTH* conf.WORLD_WIDTH),

	// Generation
	m_GenerationNoise((unsigned int)std::time(NULL), (unsigned int)std::time(NULL) + 1000, (unsigned int)std::time(NULL) + 2000),
	m_GenerationSemaphore(0),
	//Game
	m_CharacterController({ 0.f, 30.f, 0.f }),
	// Graphics
	m_ShaderPackage{ new Shader("res/shaders/block/static/shader_static.vert", "res/shaders/block/static/shader_static.frag") },
	r_Window(window),
	m_ChunkBorderRenderer(24, "res/shaders/universal/shader_single_color_instanced.vert", "res/shaders/universal/shader_single_color.frag"),
	m_BlockSelectionRenderer("res/shaders/universal/shader_single_color.vert", "res/shaders/universal/shader_single_color.frag"),
	m_HUDRenderer(1, { 3, 3 }, "res/shaders/sprite/shader_sprite.vert", "res/shaders/sprite/shader_sprite.frag")
{
	m_MatrixView = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -3.5f));
	m_MatrixProjection = glm::perspective(glm::radians(conf.FOV), (float)conf.WIN_WIDTH / (float)conf.WIN_HEIGHT, 0.1f, 300.f);

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformMat4f("u_Projection", m_MatrixProjection);

	// Experimental Texture Setup
	m_TextureMap.Bind(Minecraft::Global::SAMPLER_SLOT_BLOCKS);
	m_ShaderPackage.shaderBlockStatic->SetUniform1i("u_TextureMap", m_TextureMap.GetBoundPort());

	for (int i = 0; i < 5; i++) {
		m_BiomeTemplate.push_back(std::vector<Minecraft::Biome>(5));
	}

	// Parse blocks before textures!!!
	ParseBlocks("docs/block.yaml");
	ParseTextures("docs/texture.yaml");
	ParseStructures("docs/structure.yaml");
	ParseBiomes("docs/biome.yaml");

	for (unsigned int i = 0; i < conf.GENERATION_THREADS; i++) {
		m_GenerationThreads.push_back(std::thread([this]() {
			this->GenerationThreadJob();
			}));
	}

	m_Chunks.reserve(conf.RENDER_DISTANCE * conf.RENDER_DISTANCE * 4);
	GenerateTerrain();
	SetupLight();

	m_ChunkBorderRenderer.shader->Bind();
	m_ChunkBorderRenderer.shader->SetUniformMat4f("u_Projection", m_MatrixProjection);
	m_ChunkBorderRenderer.shader->SetUniform1f("u_ChunkWidth", (float)conf.CHUNK_SIZE);
	m_ChunkBorderRenderer.shader->SetUniform1f("u_WorldWidth", (float)conf.WORLD_WIDTH);

	m_BlockSelectionRenderer.shader->Bind();
	m_BlockSelectionRenderer.shader->SetUniformMat4f("u_Projection", m_MatrixProjection);

	SetupChunkBorders();
	m_HUDRenderer.PushSprite("res/images/hud/crossair.png", { conf.WIN_WIDTH / 2.f - 8.f, conf.WIN_HEIGHT / 2.f - 8.f }, { 16, 16 });

	m_CharacterController.setInventoryReference(&m_Inventory);
}

World::~World()
{
	LOGC("Removing Chunks", LOG_COLOR::LOG);
	for (Chunk* chunk : m_Chunks) {
		delete chunk;
	}
	m_Chunks.clear();

	// Safely exiting worker threads
	m_ExecuteGenerationJob = false;
	m_GenerationSemaphore.release(1000);
	for (auto& thread : m_GenerationThreads) {
		if (thread.joinable()) {
			auto id = thread.get_id();
			std::stringstream ss;
			ss << id;

			LOGC(("Joining Generation Thread #" + ss.str()), LOG_COLOR::OK);
			thread.join();
		}
	}
}

void World::OnRender()
{
	m_TextureMap.Bind(Minecraft::Global::SAMPLER_SLOT_BLOCKS); // Why?

	// Render opac objects
	for (Chunk* chunk : m_Chunks) {
		if (!chunk || !chunk->isLoaded()) continue;
		chunk->OnRender(m_ShaderPackage);
	}

	// Render transparent objects
	for (Chunk* chunk : m_Chunks) {
		if (!chunk) continue;
		chunk->OnRenderTransparents(m_ShaderPackage, m_CharacterController.getPosition());
	}

	if (m_DrawChunkBorder) {
		m_ChunkBorderRenderer.DrawInstanced(24, m_CountChunks);
		m_DrawCalls++;
	}

	m_BlockSelectionRenderer.Draw();
	m_Inventory.OnRender();
	m_HUDRenderer.Draw();
}

void World::NeighborChunks()
{
	for (unsigned int x = 0; x < conf.WORLD_WIDTH; x++) {
		for (unsigned int z = 0; z < conf.WORLD_WIDTH; z++) {
			Chunk* chunk = m_Chunks[CoordToChunkIndex({ x, z })];
			if (!chunk) continue;

			chunk->setChunkNeighbors(
				z > 0 ? m_Chunks[CoordToChunkIndex({ x - 0, z - 1 })] : nullptr,
				x < conf.WORLD_WIDTH - 1 ? m_Chunks[CoordToChunkIndex({ x + 1, z - 0 })] : nullptr,
				z < conf.WORLD_WIDTH - 1 ? m_Chunks[CoordToChunkIndex({ x - 0, z + 1 })] : nullptr,
				x > 0 ? m_Chunks[CoordToChunkIndex({ x - 1, z - 0 })] : nullptr
			);
		}
	}
}

void World::OnUpdate(double deltaTime)
{
	OnWindowResize();

	const glm::vec3& position = m_CharacterController.getPosition();
	m_MatrixView = glm::lookAt(position, position + m_CharacterController.getFront(), m_CharacterController.getUp());

	UpdateLight();
	HandleChunkLoading();
	m_CharacterController.OnUpdate(deltaTime);
	OutlineSelectedBlock();

	m_Inventory.OnUpdate();

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformMat4f("u_View", m_MatrixView);
	m_ShaderPackage.shaderBlockStatic->SetUniform3f("u_ViewPosition", position.x, position.y, position.z);

	m_ChunkBorderRenderer.shader->Bind();
	m_ChunkBorderRenderer.shader->SetUniformMat4f("u_View", m_MatrixView);

	m_BlockSelectionRenderer.shader->Bind();
	m_BlockSelectionRenderer.shader->SetUniformMat4f("u_View", m_MatrixView);
}

void World::OnWindowResize()
{
	if (Minecraft::Global::updateResize) {
		UpdateProjectionMatrix(conf.FOV, 0.1f, 300.f);
	}
}

void World::UpdateProjectionMatrix(float FOV, float nearD, float farD)
{
	m_MatrixProjection = glm::perspective(glm::radians(FOV), (float)Minecraft::Global::windowSize.x / (float)Minecraft::Global::windowSize.y, nearD, farD);

	m_ShaderPackage.shaderBlockStatic->Bind();
	m_ShaderPackage.shaderBlockStatic->SetUniformMat4f("u_Projection", m_MatrixProjection);

	m_ChunkBorderRenderer.shader->Bind();
	m_ChunkBorderRenderer.shader->SetUniformMat4f("u_Projection", m_MatrixProjection);

	m_BlockSelectionRenderer.shader->Bind();
	m_BlockSelectionRenderer.shader->SetUniformMat4f("u_Projection", m_MatrixProjection);
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
	return m_CharacterController.getPosition();
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
	m_Inventory.OnInput(window);
	const glm::vec3& position = m_CharacterController.getPosition();
	glm::vec2 currentPLayerChunkPosition = { std::floor(abs(m_WorldRootPosition.x - position.x) / conf.CHUNK_SIZE), std::floor(abs(m_WorldRootPosition.z - position.z) / conf.CHUNK_SIZE) };

	Chunk* chunks[9];
	unsigned int offset = 0;
	for (int x = 0; x < 3; x++) {
		for (int z = 0; z < 3; z++) {
			chunks[offset++] = CoordToChunkSecure({ currentPLayerChunkPosition.x - 1 + x, currentPLayerChunkPosition.y - 1 + z });
		}
	}

	Minecraft::CharacterController::InputChangeEvent event = m_CharacterController.OnInput(window, deltaTime, chunks);
	if (event.doChangeFOV) UpdateProjectionMatrix(event.FOV);

	if (event.chunkToBeQueued) m_ChunksQueuedCulling.push_back(event.chunkToBeQueued);
	m_GenerationSemaphore.release(event.threadJobs);
}

void World::GenerateTerrain()
{
	LOGC("Generating Terrain", LOG_COLOR::SPECIAL_B);
	unsigned int chunkWidth = (unsigned int)(conf.CHUNK_SIZE * conf.BLOCK_SIZE);

	glm::vec2 rootPosition = { conf.WORLD_WIDTH / 2, conf.WORLD_WIDTH / 2 };															// Spawn Chunk in Matrix Space
	glm::vec2 generationPosition = { conf.WORLD_WIDTH / 2 - conf.RENDER_DISTANCE, conf.WORLD_WIDTH / 2 - conf.RENDER_DISTANCE };		// Upper left edge of Spawn area in Matrix Space
	m_WorldRootPosition = { -(int)((rootPosition.x + 0.5) * chunkWidth), 0, -(int)((rootPosition.y + 0.5) * chunkWidth) };				// Upper left edge of World in World Space

	glm::vec3 chunkOffset = { 0.f, 0.f, 0.f };
	for (unsigned int x = 0; x < conf.RENDER_DISTANCE * 2 + 1; x++) {
		chunkOffset.z = 0.f;
		for (unsigned int z = 0; z < conf.RENDER_DISTANCE * 2 + 1; z++) {
			const unsigned int i = CoordToChunkIndex(generationPosition + glm::vec2{ x, z });
			LOGC(("Chunk " + std::to_string(i) + " queued"), LOG_COLOR::LOG);
			m_Chunks[i] = new Chunk(&m_BlockFormats, &m_TextureFormats);
			m_Chunks[i]->setID(i);
			m_Chunks[i]->setGenerationData({ (-(int)((conf.RENDER_DISTANCE + 0.5) * chunkWidth)) + chunkOffset.x, 0, (-(int)((conf.RENDER_DISTANCE + 0.5) * chunkWidth)) + chunkOffset.z },
				{ (generationPosition.x + x) * 1.f, (generationPosition.y + z) * 1.f, 1.f },
				{ (generationPosition.x) * conf.NOISE_SIZE_TEMP + x, (generationPosition.y) * conf.NOISE_SIZE_TEMP + z, 1.f },
				{ (generationPosition.x) * conf.NOISE_SIZE_MOIST + x, (generationPosition.y) * conf.NOISE_SIZE_MOIST + z, 1.f }
			, &m_GenerationNoise);

			m_Chunks[i]->setBiomeTemplate(&m_BiomeTemplate);
			m_Chunks[i]->setStructureTemplate(&m_StructureTemplate);

			if (x == conf.RENDER_DISTANCE && z == conf.RENDER_DISTANCE) m_Chunks[i]->setSpawnFlag();

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

		Chunk* c1 = coord.y > 0 ? m_Chunks[CoordToChunkIndex({ coord.x + 0, coord.y - 1 })] : nullptr;
		Chunk* c2 = coord.x < conf.WORLD_WIDTH - 1 ? m_Chunks[CoordToChunkIndex({ coord.x + 1, coord.y + 0 })] : nullptr;
		Chunk* c3 = coord.y < conf.WORLD_WIDTH - 1 ? m_Chunks[CoordToChunkIndex({ coord.x + 0, coord.y + 1 })] : nullptr;
		Chunk* c4 = coord.x > 0 ? m_Chunks[CoordToChunkIndex({ coord.x - 1, coord.y + 0 })] : nullptr;

		m_Chunks[i]->setChunkNeighbors(c1, c2, c3, c4);
	}

	if (conf.ENABLE_MULTITHREADING) m_GenerationSemaphore.release(m_ChunksQueuedGenerating.size());
}

inline Chunk* World::CoordToChunkSecure(const glm::vec2& coord)
{
	int index = CoordToChunkIndex(coord);
	if (index == -1) return nullptr;
	else {
		return m_Chunks[index];
	}
}

inline int World::CoordToChunkIndex(const glm::vec2& coord) const
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
	LOGC("Started generation Thread", LOG_COLOR::LOG);
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

				if (chunk->isSpawnChunk()) {
					int y = 0;
					while (chunk->getBlock({ conf.CHUNK_SIZE / 2.f, y++, conf.CHUNK_SIZE / 2.f })) {}
					m_CharacterController.Spawn({ 0.f, --y, 0.f });
				}

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
		LOGC("Warning: Threading skipped a task!", LOG_COLOR::WARNING);
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

	const glm::vec3& position = m_CharacterController.getPosition();
	glm::vec2 currentPLayerChunkPosition = { std::floor(abs(m_WorldRootPosition.x - position.x) / conf.CHUNK_SIZE), std::floor(abs(m_WorldRootPosition.z - position.z) / conf.CHUNK_SIZE) };

	if (m_IsGenerationInit) {
		m_PlayerChunkPosition = currentPLayerChunkPosition;
		m_IsGenerationInit = false;
	}

	if (currentPLayerChunkPosition != m_PlayerChunkPosition && conf.EXPAND_TERRAIN) {
		hasExpanded = true;
		LOGC("expanded terrain", LOG_COLOR::SPECIAL_A);

		for (unsigned int x = 0; x < conf.WORLD_WIDTH; x++) {
			for (unsigned int z = 0; z < conf.WORLD_WIDTH; z++) {
				unsigned int index = CoordToChunkIndex({ x, z });
				if (index == -1) continue;

				if (abs(x - currentPLayerChunkPosition.x) <= conf.RENDER_DISTANCE &&
					abs(z - currentPLayerChunkPosition.y) <= conf.RENDER_DISTANCE) {
					if (m_Chunks[index] && !m_Chunks[index]->isLoaded()) {
						m_Chunks[index]->LoadVertexBufferFromLoadBuffer();
					}
					else if (!m_Chunks[index]) {
						Chunk* chunk = new Chunk(&m_BlockFormats, &m_TextureFormats);
						m_Chunks[index] = chunk;
						chunk->setID(index);
						chunk->setGenerationData({ m_WorldRootPosition.x + x * conf.CHUNK_SIZE, 0, m_WorldRootPosition.z + z * conf.CHUNK_SIZE },
							{ x * 1.f, z * 1.f, 1.f }, { x * conf.NOISE_SIZE_TEMP, z * conf.NOISE_SIZE_TEMP, 1.f }, { x * conf.NOISE_SIZE_MOIST, z * conf.NOISE_SIZE_MOIST, 1.f }, &m_GenerationNoise);
						chunk->setBiomeTemplate(&m_BiomeTemplate);

						index = CoordToChunkIndex({ x - 1, z });
						if (index != -1 && m_Chunks[index] && m_Chunks[index]->IsGenerated()) ChunksReculled.insert(m_Chunks[index]);

						index = CoordToChunkIndex({ x + 1, z });
						if (index != -1 && m_Chunks[index] && m_Chunks[index]->IsGenerated()) ChunksReculled.insert(m_Chunks[index]);

						index = CoordToChunkIndex({ x, z - 1 });
						if (index != -1 && m_Chunks[index] && m_Chunks[index]->IsGenerated()) ChunksReculled.insert(m_Chunks[index]);

						index = CoordToChunkIndex({ x, z + 1 });
						if (index != -1 && m_Chunks[index] && m_Chunks[index]->IsGenerated()) ChunksReculled.insert(m_Chunks[index]);

						m_MutexGenerating.lock();
						m_ChunksQueuedGenerating.push_back(chunk);
						waitingForNeighboring++;
						m_MutexGenerating.unlock();
					}
				}
				else if (m_Chunks[index] && m_Chunks[index]->isLoaded()) {
					m_Chunks[index]->Unload();
				}
			}
		}
	}

	if (hasExpanded) NeighborChunks();

	for (auto it = ChunksReculled.begin(); it != ChunksReculled.end(); it++) {
		m_ChunksQueuedCulling.push_back(*it);
	}

	m_GenerationSemaphore.release(waitingForNeighboring + ChunksReculled.size());
	m_PlayerChunkPosition = currentPLayerChunkPosition;
}

void World::OutlineSelectedBlock()
{
	const Minecraft::Block_static* selectedBlock = m_CharacterController.getSelectedBlock();
	if (selectedBlock) m_BlockSelectionRenderer.LoadOutlineBuffer(selectedBlock->position - glm::vec3(0.f, 0.f, 0.f), conf.BLOCK_SIZE);
	else m_BlockSelectionRenderer.DoNotDraw();
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
	LOGC("Parsing Blocklist", LOG_COLOR::SPECIAL_A);
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
	LOGC("Parsing Texturelist", LOG_COLOR::SPECIAL_A);
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

void World::ParseBiomes(const std::string& path)
{
	LOGC("Parsing Biomes", LOG_COLOR::SPECIAL_A);
	YAML::Node mainNode = YAML::LoadFile(path);

	unsigned int count = 0;
	for (auto biomeEntry : mainNode) {
		Minecraft::Biome biome;

		biome.name = biomeEntry.first.as<std::string>();
		biome.id = count++;

		for (int i = 0; i < biomeEntry.second["structures"].size(); i++) {
			int structure = biomeEntry.second["structures"][i][0].as<int>();
			if (structure != -1) {
				biome.structures.push_back(structure);
				biome.structureProb.push_back(biomeEntry.second["structures"][i][1].as<float>());
			}
		}

		for (int i = 0; i < biomeEntry.second["blocks"].size(); i++) {
			biome.blocks.push_back(biomeEntry.second["blocks"][i].as<int>());
		}

		for (int i = 0; i < biomeEntry.second["moist"].size(); i++) {
			for (int j = 0; j < biomeEntry.second["temp"].size(); j++) {
				int x = biomeEntry.second["moist"][i].as<int>();
				int y = biomeEntry.second["temp"][j].as<int>();
				m_BiomeTemplate[x][y] = biome;
			}
		}
	}
}

void World::ParseStructures(const std::string& path)
{
	LOGC("Parsing Structures", LOG_COLOR::SPECIAL_A);
	YAML::Node mainNode = YAML::LoadFile(path);

	unsigned int count = 0;
	for (auto structureEntry : mainNode) {
		Minecraft::Structure structure;

		structure.name = structureEntry.first.as<std::string>();
		structure.id = structureEntry.second["id"].as<unsigned int>();

		for (int i = 0; i < structureEntry.second["blocks"].size(); i++) {
			glm::vec4 block{};
			block.x = structureEntry.second["blocks"][i][0].as<int>();
			block.y = structureEntry.second["blocks"][i][1].as<int>();
			block.z = structureEntry.second["blocks"][i][2].as<int>();
			block.a = structureEntry.second["blocks"][i][3].as<int>();
			structure.blocks.push_back(block);
		}

		m_StructureTemplate.push_back(structure);
	}
}

void World::SetupChunkBorders()
{
	float chunkWidth = conf.BLOCK_SIZE * conf.CHUNK_SIZE;
	float chunkHeight = conf.BLOCK_SIZE * conf.CHUNK_HEIGHT;
	Minecraft::PositionVertex v[24]{};

	v[0].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + chunkWidth };
	v[1].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + chunkHeight,	m_WorldRootPosition.z + chunkWidth };
	v[2].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + 0 };
	v[3].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + chunkHeight,	m_WorldRootPosition.z + 0 };

	v[4].Position = { m_WorldRootPosition.x + chunkWidth,	m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + 0 };
	v[5].Position = { m_WorldRootPosition.x + chunkWidth,	m_WorldRootPosition.y + chunkHeight,	m_WorldRootPosition.z + 0 };
	v[6].Position = { m_WorldRootPosition.x + chunkWidth,	m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + chunkWidth };
	v[7].Position = { m_WorldRootPosition.x + chunkWidth,	m_WorldRootPosition.y + chunkHeight,	m_WorldRootPosition.z + chunkWidth };

	v[8].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + chunkWidth };
	v[9].Position = { m_WorldRootPosition.x + chunkWidth,	m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + chunkWidth };
	v[10].Position = { m_WorldRootPosition.x + chunkWidth, m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + chunkWidth };
	v[11].Position = { m_WorldRootPosition.x + chunkWidth, m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + 0 };

	v[12].Position = { m_WorldRootPosition.x + chunkWidth, m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + 0 };
	v[13].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + 0 };
	v[14].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + 0 };
	v[15].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + 0,				m_WorldRootPosition.z + chunkWidth };

	v[16].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + chunkHeight,	m_WorldRootPosition.z + chunkWidth };
	v[17].Position = { m_WorldRootPosition.x + chunkWidth,					m_WorldRootPosition.y + chunkHeight, m_WorldRootPosition.z + chunkWidth };
	v[18].Position = { m_WorldRootPosition.x + chunkWidth,					m_WorldRootPosition.y + chunkHeight, m_WorldRootPosition.z + chunkWidth };
	v[19].Position = { m_WorldRootPosition.x + chunkWidth,					m_WorldRootPosition.y + chunkHeight, m_WorldRootPosition.z + 0 };

	v[20].Position = { m_WorldRootPosition.x + chunkWidth, m_WorldRootPosition.y + chunkHeight,	m_WorldRootPosition.z + 0 };
	v[21].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + chunkHeight,	m_WorldRootPosition.z + 0 };
	v[22].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + chunkHeight,	m_WorldRootPosition.z + 0 };
	v[23].Position = { m_WorldRootPosition.x + 0,			m_WorldRootPosition.y + chunkHeight,	m_WorldRootPosition.z + chunkWidth };

	m_ChunkBorderRenderer.vb->AddVertexData(v, sizeof(Minecraft::PositionVertex) * 24);
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