#pragma once

#include <yaml-cpp/yaml.h>

#include <iostream>
#define LOG(message) std::cout << message << std::endl
#define ASSERT(x) if((x)) __debugbreak();

#include <string>
class Config {
public:
	Config(const std::string& path) {
		LOG("Parsing Config");
		YAML::Node mainNode = YAML::LoadFile(path);

		c_WIN_WIDTH = mainNode["config"]["window"]["Width"].as<unsigned int>();
		c_WIN_HEIGHT = mainNode["config"]["window"]["Height"].as<unsigned int>();

		c_MAX_BUFFER_FACES = mainNode["config"]["rendering"]["MaxBufferFaces"].as<unsigned int>();
		c_TEXTURE_SIZE = mainNode["config"]["rendering"]["TextureSize"].as<unsigned int>();
		c_RENDER_DISTANCE = mainNode["config"]["rendering"]["RenderDistance"].as<unsigned int>();
		c_TEXTURE_INVERSE_OFFSET = mainNode["config"]["rendering"]["TextureInverseOffset"].as<unsigned int>();

		c_CHUNK_SIZE = mainNode["config"]["game"]["terrain"]["ChunkSize"].as<unsigned int>();
		c_BLOCK_SIZE = mainNode["config"]["game"]["terrain"]["BlockSize"].as<float>();
		c_CHUNK_HEIGHT = mainNode["config"]["game"]["terrain"]["ChunkHeight"].as<unsigned int>();
		c_TERRAIN_STRETCH_Y = mainNode["config"]["game"]["terrain"]["YStretch"].as<float>();
		c_TERRAIN_STRETCH_X = mainNode["config"]["game"]["terrain"]["XStretch"].as<float>();
		c_TERRAIN_MIN_HEIGHT = mainNode["config"]["game"]["terrain"]["MinHeight"].as<unsigned int>();

		c_FOG_AFFECT_DISTANCE = mainNode["config"]["game"]["environment"]["FogAffectDistance"].as<float>();
		c_FOG_DENSITY = mainNode["config"]["game"]["environment"]["FogDensity"].as<float>();

		c_CHUNK_VOLUME = c_CHUNK_HEIGHT * c_CHUNK_SIZE * c_CHUNK_SIZE;
	}

	// Window
	unsigned int c_WIN_WIDTH = 0;
	unsigned int c_WIN_HEIGHT = 0;

	// Rendering
	unsigned int c_MAX_BUFFER_FACES = 0;
	unsigned int c_TEXTURE_SIZE = 0;
	unsigned int c_RENDER_DISTANCE = 0;
	unsigned int c_TEXTURE_INVERSE_OFFSET = 0;

	// Game
	// Terrain
	unsigned int c_CHUNK_SIZE = 0;
	float c_BLOCK_SIZE = 0;
	unsigned int c_CHUNK_HEIGHT = 0;
	unsigned int c_CHUNK_VOLUME = 0;

	float c_TERRAIN_STRETCH_Y = 0;
	float c_TERRAIN_STRETCH_X = 0;

	unsigned int c_TERRAIN_MIN_HEIGHT = 0;

	// Environment
	float c_FOG_AFFECT_DISTANCE = 0;
	float c_FOG_DENSITY = 0;
};

extern Config conf;