#include "TexturePacker.h"

int TexturePacker::roundUp(int numToRound, int multiple)
{
	if (multiple == 0)
		return numToRound;

	int remainder = numToRound % multiple;
	if (remainder == 0)
		return numToRound;

	return numToRound + multiple - remainder;
}

inline int TexturePacker::CoordinateToIndex(int x, int y, int width)
{
	return y * width + x;
}

const bool TexturePacker::PackTextures(const std::string& dirPath, const std::string& sheetPath, const std::string& yamlPath, const float accur)
{
	// Color Console output
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 11);

	// Get amount of images
	unsigned int amountFiles = 0;

	for (auto image : std::filesystem::directory_iterator(dirPath)) {
		amountFiles++;
	}

	unsigned int pngOutWidth = roundUp((int)sqrt(amountFiles * conf.TEXTURE_SIZE * conf.TEXTURE_SIZE), conf.TEXTURE_SIZE);
	unsigned int pngOutHeight = 0;

	Minecraft::Image::Pixel* imageBuffer = (Minecraft::Image::Pixel*)std::malloc(pngOutWidth * pngOutWidth * sizeof(Minecraft::Image::Pixel));
	int width, height, channels;
	char* img = nullptr;

	std::vector<std::string> largeImages;

	unsigned int offsetX = 0, offsetY = 0;
	char const* out = sheetPath.c_str();
	int lineHeight = 0;

	YAML::Node node;

	for (auto image : std::filesystem::directory_iterator(dirPath)) {
		const std::string& path = image.path().string();
		img = (char*)stbi_load(path.c_str(), &width, &height, &channels, 0);
		
		if (img) {
			std::cout << "Loaded " << path << "." << '\n';

			if (height > width) {
				largeImages.push_back(path);
				free(img);
				continue;
			}
			else {
				if (offsetX + width > pngOutWidth) {
					offsetX = 0;
					offsetY += lineHeight;
				}

				const std::string& filenameEXT = image.path().filename().string();
				const std::string& filename = filenameEXT.substr(0, filenameEXT.find_last_of("."));

				float x0 = std::ceil(((float)offsetX / pngOutWidth) * accur) / accur;
				float x1 = std::floor((((float)offsetX + width) / pngOutWidth) * accur) / accur;

				float y0 = std::ceil(((float)offsetY / pngOutWidth) * accur) / accur;
				float y1 = std::floor((((float)offsetY + height) / pngOutWidth) * accur) / accur;

				node[filename]["uvs"]["0"].push_back(x0);
				node[filename]["uvs"]["0"].push_back(y1);

				node[filename]["uvs"]["1"].push_back(x1);
				node[filename]["uvs"]["1"].push_back(y1);

				node[filename]["uvs"]["2"].push_back(x1);
				node[filename]["uvs"]["2"].push_back(y0);

				node[filename]["uvs"]["3"].push_back(x0);
				node[filename]["uvs"]["3"].push_back(y0);

				if (height > lineHeight) lineHeight = height;

				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						char* pixelPtr = &img[(y * height + x) * 4u];
						imageBuffer[CoordinateToIndex(offsetX + x, offsetY + y, pngOutWidth)] = Minecraft::Image::Pixel{ *(pixelPtr + 0), *(pixelPtr + 1), *(pixelPtr + 2), *(pixelPtr + 3) };
					}
				}
				offsetX += width;
			}
		}
		else {
			SetConsoleTextAttribute(hConsole, 12);
			std::cout << "Could not load " << path << "." << '\n';
		}

		free(img);
	}

	// Pack large images at the bottom
	for (const std::string& p : largeImages) {

	}

	// Write Image
	stbi_write_png(out, pngOutWidth, pngOutWidth, channels, &imageBuffer->r, sizeof(Minecraft::Image::Pixel) * pngOutWidth);
	free(imageBuffer);

	// Write Texture Yaml
	std::ofstream fout(yamlPath);
	fout << node;
	fout.close();

	SetConsoleTextAttribute(hConsole, 15);
	return false;
}
