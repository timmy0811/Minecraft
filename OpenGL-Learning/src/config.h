#pragma once

#include "windowsWrapper.h"
#include <yaml-cpp/yaml.h>
#include "glm/glm.hpp"

#include <string>
#include <iostream>

#define LOG(message) std::cout << message << std::endl
#define ASSERT(x) if((x)) __debugbreak();

enum class LOG_COLOR { LOG = 15, WARNING = 14, OK = 10, FAULT = 12, SPECIAL_A = 11, SPECIAL_B = 13 };

static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
inline void LOGC(const std::string& msg, LOG_COLOR color = LOG_COLOR::LOG) {
	SetConsoleTextAttribute(hConsole, (int)color);
	std::cout << msg << '\n';
	SetConsoleTextAttribute(hConsole, 15);
}

namespace Minecraft::Global {
	inline int TEXTURE_BINDING = 0;

	inline int SAMPLER_SLOT_BLOCKS = 1;
	inline int SAMPLER_SLOT_SKYBOX = 2;
	inline int SAMPLER_SLOT_SPRITES = 3;
	inline int SAMPLER_SLOT_FONTS = 9;
	inline int SAMPLER_SLOT_SHADOWMAP = 12;
	inline bool fontbound = false;

	inline glm::ivec2 windowSize;
	inline bool updateResize = true;
}

class Config {
private:
	const std::string m_Path;
public:
	Config(const std::string& path)
		:m_Path(path)
	{
		Parse();
		Validate();
	}

	void Parse() {
		LOGC("Parsing Config", LOG_COLOR::SPECIAL_A);
		YAML::Node mainNode = YAML::LoadFile(m_Path);

		WIN_WIDTH = mainNode["config"]["window"]["Width"].as<unsigned int>();
		WIN_HEIGHT = mainNode["config"]["window"]["Height"].as<unsigned int>();

		FOV = mainNode["config"]["rendering"]["FOV"].as<float>();
		MAX_BUFFER_FACES = mainNode["config"]["rendering"]["MaxBufferFaces"].as<unsigned int>();
		TEXTURE_SIZE = mainNode["config"]["rendering"]["TextureSize"].as<unsigned int>();
		RENDER_DISTANCE = mainNode["config"]["rendering"]["RenderDistance"].as<unsigned int>();
		TEXTURE_INVERSE_OFFSET = mainNode["config"]["rendering"]["TextureInverseOffset"].as<float>();
		TEXTURE_PACK = mainNode["config"]["rendering"]["TexturePack"].as<std::string>();
		EXPAND_TERRAIN = mainNode["config"]["rendering"]["ExpandTerrain"].as<bool>();

		CHUNK_BORDER_COLOR.r = mainNode["config"]["rendering"]["ChunkBorderColor"]["r"].as<float>();
		CHUNK_BORDER_COLOR.g = mainNode["config"]["rendering"]["ChunkBorderColor"]["g"].as<float>();
		CHUNK_BORDER_COLOR.b = mainNode["config"]["rendering"]["ChunkBorderColor"]["b"].as<float>();
		CHUNK_BORDER_COLOR.a = mainNode["config"]["rendering"]["ChunkBorderColor"]["a"].as<float>();

		SHADOW_MAP_SIZE = mainNode["config"]["rendering"]["shadows"]["MapSize"].as<unsigned int>();
		BIND_SHADOW_MAP_PLAYER_POS = mainNode["config"]["rendering"]["shadows"]["BindMapToPlayerPos"].as<bool>();

		WORLD_WIDTH = mainNode["config"]["game"]["terrain"]["WorldWidth"].as<unsigned int>();
		if (WORLD_WIDTH % 2 != 0) WORLD_WIDTH += 1;

		CHUNK_SIZE = mainNode["config"]["game"]["terrain"]["ChunkSize"].as<unsigned int>();
		BLOCK_SIZE = mainNode["config"]["game"]["terrain"]["BlockSize"].as<float>();
		CHUNK_HEIGHT = mainNode["config"]["game"]["terrain"]["ChunkHeight"].as<unsigned int>();
		TERRAIN_STRETCH_Y = mainNode["config"]["game"]["terrain"]["YStretch"].as<float>();
		TERRAIN_STRETCH_X = mainNode["config"]["game"]["terrain"]["XStretch"].as<float>();
		TERRAIN_MIN_HEIGHT = mainNode["config"]["game"]["terrain"]["MinHeight"].as<unsigned int>();

		NOISE_SIZE_MOIST = mainNode["config"]["game"]["terrain"]["NoiseSizeMoist"].as<float>();
		NOISE_SIZE_TEMP = mainNode["config"]["game"]["terrain"]["NoiseSizeTemp"].as<float>();

		FOG_AFFECT_DISTANCE = mainNode["config"]["game"]["environment"]["FogAffectDistance"].as<float>();
		FOG_DENSITY = mainNode["config"]["game"]["environment"]["FogDensity"].as<float>();
		FOG_COLOR.r = mainNode["config"]["game"]["environment"]["FogColor"]["r"].as<float>();
		FOG_COLOR.g = mainNode["config"]["game"]["environment"]["FogColor"]["g"].as<float>();
		FOG_COLOR.b = mainNode["config"]["game"]["environment"]["FogColor"]["b"].as<float>();

		MOVEMENT_GRAVITATION = mainNode["config"]["game"]["movement"]["Gravitation"].as<float>();
		MOVEMENT_PLAYER_DRAG = mainNode["config"]["game"]["movement"]["PlayerDrag"].as<float>();
		MOVEMENT_PLAYER_DRAG_AIR = mainNode["config"]["game"]["movement"]["PlayerDragInAir"].as<float>();
		MOVEMENT_CONTROLL_AIR = mainNode["config"]["game"]["movement"]["ControllInAir"].as<float>();

		MOVEMENT_SPEED_WALK = mainNode["config"]["game"]["movement"]["SpeedWalk"].as<float>();
		MOVEMENT_SPEED_SPRINT = mainNode["config"]["game"]["movement"]["SpeedSprint"].as<float>();
		MOVEMENT_SPEED_CROUCH = mainNode["config"]["game"]["movement"]["SpeedCrouch"].as<float>();
		MOVEMENT_MAX_FALL_SPEED = mainNode["config"]["game"]["movement"]["MaxFallSpeed"].as<float>();
		MOVEMENT_JUMP_STRENGHT = mainNode["config"]["game"]["movement"]["JumpStrenght"].as<float>();

		FOV_CROUCH = mainNode["config"]["game"]["movement"]["FOVcrouching"].as<float>();
		FOV_SPRINT = mainNode["config"]["game"]["movement"]["FOVsprinting"].as<float>();
		FOV_WALK = mainNode["config"]["game"]["movement"]["FOVwalking"].as<float>();
		BLOCK_INTERACTION_RANGE = mainNode["config"]["game"]["movement"]["BlockInteractionRange"].as<float>();

		ENABLE_MULTITHREADING = mainNode["config"]["system"]["EnableMultithreading"].as<bool>();
		GENERATION_THREADS = mainNode["config"]["system"]["GenerationThreads"].as<unsigned int>();

		GUI_SCALE = mainNode["config"]["game"]["gui"]["scale"].as<float>();

		CHUNK_VOLUME = CHUNK_HEIGHT * CHUNK_SIZE * CHUNK_SIZE;
	}

	void Validate() {
		if (CHUNK_HEIGHT < TERRAIN_MIN_HEIGHT + TERRAIN_STRETCH_Y - 1) LOGC("Warning: Terrain Exceeding Chunk Boundaries!", LOG_COLOR::WARNING);
	}

	// System
	bool ENABLE_MULTITHREADING = false;
	unsigned int GENERATION_THREADS = 0;

	// Window
	unsigned int WIN_WIDTH = 0;
	unsigned int WIN_HEIGHT = 0;

	// Rendering
	float FOV = 0.f;
	unsigned int MAX_BUFFER_FACES = 0;
	unsigned int TEXTURE_SIZE = 0;
	std::string TEXTURE_PACK = "";
	unsigned int RENDER_DISTANCE = 0;
	float TEXTURE_INVERSE_OFFSET = 0.f;
	bool EXPAND_TERRAIN = false;

	glm::vec4 CHUNK_BORDER_COLOR = { 0.f, 0.f, 0.f, 1.f };

	unsigned int SHADOW_MAP_SIZE = 1024;
	bool BIND_SHADOW_MAP_PLAYER_POS = false;

	// Game
	// Terrain
	unsigned int WORLD_WIDTH = 0;
	unsigned int CHUNK_SIZE = 0;
	float BLOCK_SIZE = 0;
	unsigned int CHUNK_HEIGHT = 0;
	unsigned int CHUNK_VOLUME = 0;

	float TERRAIN_STRETCH_Y = 0;
	float TERRAIN_STRETCH_X = 0;

	unsigned int TERRAIN_MIN_HEIGHT = 0;

	float NOISE_SIZE_MOIST = 0.f;
	float NOISE_SIZE_TEMP = 0.f;

	// Environment
	float FOG_AFFECT_DISTANCE = 0;
	float FOG_DENSITY = 0;
	glm::vec3 FOG_COLOR = { 0.f, 0.f, 0.f };

	// Movement
	float MOVEMENT_GRAVITATION = 0.f;
	float MOVEMENT_PLAYER_DRAG = 0.f;
	float MOVEMENT_PLAYER_DRAG_AIR = 0.f;
	float MOVEMENT_CONTROLL_AIR = 0.f;

	float MOVEMENT_SPEED_WALK = 0.f;
	float MOVEMENT_SPEED_SPRINT = 0.f;
	float MOVEMENT_SPEED_CROUCH = 0.f;
	float MOVEMENT_MAX_FALL_SPEED = 0.f;
	float MOVEMENT_JUMP_STRENGHT = 0.f;

	float FOV_CROUCH = 0.f;
	float FOV_WALK = 0.f;
	float FOV_SPRINT = 0.f;

	float BLOCK_INTERACTION_RANGE = 0.f;

	// GUI
	float GUI_SCALE = 1.f;
};

extern Config conf;