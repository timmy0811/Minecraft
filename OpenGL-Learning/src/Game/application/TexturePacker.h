#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <list>

#include "vendor/stb_image/stb_image_write.h"

#include "config.h"

namespace Minecraft::Image {
	struct Pixel {
		char r, g, b, a;
	};
}

class TexturePacker
{
private:
	static std::vector<std::string> m_PackedTextures;	

public:
	static const bool PackTextures(const std::string& path);
};

