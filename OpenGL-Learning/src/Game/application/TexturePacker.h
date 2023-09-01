#pragma once

#include <vector>
#include <map>
#include <string>
#include <filesystem>
#include <iostream>
#include <list>
#include "windowsWrapper.h"

#include "vendor/stb_image/stb_image.h"
#include "vendor/stb_image/stb_image_write.h"

#include "vendor/yaml/yaml_wrapper.hpp"

#include "config.h"

namespace Minecraft::Image {
	struct Pixel {
		char r, g, b, a;
	};

	enum class TextureType { ALBEDO, NORMAL, SPECULAR };
}

class TexturePacker
{
public:
	static const bool PackTextures(const std::string& dirPath, const std::string& sheetPath, const std::string& yamlPath, const float shrinkInwards = 0.f, Minecraft::Image::TextureType imgType = Minecraft::Image::TextureType::ALBEDO);

private:
	static std::vector<std::string> m_PackedTextures;
	static int roundUp(int numToRound, int multiple);
	inline static int CoordinateToIndex(int x, int y, int width);
	inline std::string&& getPNGSuffix(const std::string& path);
};
