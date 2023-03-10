#pragma once

#include "windowsWrapper.h"
#include <yaml-cpp/yaml.h>
#include "glm/glm.hpp"

#include <iostream>
#define LOG(message) std::cout << message << std::endl
#define ASSERT(x) if((x)) __debugbreak();

enum class LOG_COLOR { LOG = 15, WARNING = 14, OK = 10, FAULT = 14, SPECIAL_A = 11, SPECIAL_B = 13};

static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
inline void LOGC(const std::string & msg, LOG_COLOR color = LOG_COLOR::LOG) {
	SetConsoleTextAttribute(hConsole, (int)color);
	std::cout << msg << '\n';
	SetConsoleTextAttribute(hConsole, 15);
}

#include <string>
class Config {
public:
	Config(const std::string& path) {
		LOGC("Parsing Config", LOG_COLOR::LOG);
		YAML::Node mainNode = YAML::LoadFile(path);

		WIN_WIDTH = mainNode["config"]["window"]["Width"].as<unsigned int>();
		WIN_HEIGHT = mainNode["config"]["window"]["Height"].as<unsigned int>();

		MAX_BUFFER_FACES = mainNode["config"]["rendering"]["MaxBufferFaces"].as<unsigned int>();
		TEXTURE_SIZE = mainNode["config"]["rendering"]["TextureSize"].as<unsigned int>();
		RENDER_DISTANCE = mainNode["config"]["rendering"]["RenderDistance"].as<unsigned int>();
		TEXTURE_INVERSE_OFFSET = mainNode["config"]["rendering"]["TextureInverseOffset"].as<unsigned int>();
		EXPAND_TERRAIN = mainNode["config"]["rendering"]["ExpandTerrain"].as<bool>();

		CHUNK_BORDER_COLOR.r = mainNode["config"]["rendering"]["ChunkBorderColor"]["r"].as<float>();
		CHUNK_BORDER_COLOR.g = mainNode["config"]["rendering"]["ChunkBorderColor"]["g"].as<float>();
		CHUNK_BORDER_COLOR.b = mainNode["config"]["rendering"]["ChunkBorderColor"]["b"].as<float>();

		WORLD_WIDTH = mainNode["config"]["game"]["terrain"]["WorldWidth"].as<unsigned int>();
		if (WORLD_WIDTH % 2 != 0) WORLD_WIDTH += 1;

		CHUNK_SIZE = mainNode["config"]["game"]["terrain"]["ChunkSize"].as<unsigned int>();
		BLOCK_SIZE = mainNode["config"]["game"]["terrain"]["BlockSize"].as<float>();
		CHUNK_HEIGHT = mainNode["config"]["game"]["terrain"]["ChunkHeight"].as<unsigned int>();
		TERRAIN_STRETCH_Y = mainNode["config"]["game"]["terrain"]["YStretch"].as<float>();
		TERRAIN_STRETCH_X = mainNode["config"]["game"]["terrain"]["XStretch"].as<float>();
		TERRAIN_MIN_HEIGHT = mainNode["config"]["game"]["terrain"]["MinHeight"].as<unsigned int>();

		FOG_AFFECT_DISTANCE = mainNode["config"]["game"]["environment"]["FogAffectDistance"].as<float>();
		FOG_DENSITY = mainNode["config"]["game"]["environment"]["FogDensity"].as<float>();
		FOG_COLOR.r = mainNode["config"]["game"]["environment"]["FogColor"]["r"].as<float>();
		FOG_COLOR.g = mainNode["config"]["game"]["environment"]["FogColor"]["g"].as<float>();
		FOG_COLOR.b = mainNode["config"]["game"]["environment"]["FogColor"]["b"].as<float>();
		
		ENABLE_MULTITHREADING = mainNode["config"]["system"]["EnableMultithreading"].as<bool>();
		GENERATION_THREADS = mainNode["config"]["system"]["GenerationThreads"].as<unsigned int>();

		CHUNK_VOLUME = CHUNK_HEIGHT * CHUNK_SIZE * CHUNK_SIZE;
	}

	// System
	bool ENABLE_MULTITHREADING = false;
	unsigned int GENERATION_THREADS = 0;
	
	// Window
	unsigned int WIN_WIDTH = 0;
	unsigned int WIN_HEIGHT = 0;

	// Rendering
	unsigned int MAX_BUFFER_FACES = 0;
	unsigned int TEXTURE_SIZE = 0;
	unsigned int RENDER_DISTANCE = 0;
	unsigned int TEXTURE_INVERSE_OFFSET = 0;
	bool EXPAND_TERRAIN = false;

	glm::vec3 CHUNK_BORDER_COLOR = { 0.f, 0.f, 0.f };

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

	// Environment
	float FOG_AFFECT_DISTANCE = 0;
	float FOG_DENSITY = 0;
	glm::vec3 FOG_COLOR = {0.f, 0.f, 0.f};
};

extern Config conf;