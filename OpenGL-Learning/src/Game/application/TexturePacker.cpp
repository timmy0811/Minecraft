#include "TexturePacker.h"

const bool TexturePacker::PackTextures(const std::string& path)
{
	// Get amount of images
	unsigned int amountFiles = 0;

	for (auto image : std::filesystem::directory_iterator(path)) {
		amountFiles++;
	}

	unsigned int pngOutWidth = sqrt(amountFiles * c_TextureSize * c_TextureSize);
	std::list<Minecraft::Image::Pixel> pixelBuffer{pngOutWidth * pngOutWidth};

	return false;
}
