#include "fileWizard.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include <string>

std::vector<float> FileWizard::Image::getPixel(const int x, const int y) const {
	std::vector<float> pixel;
	const int index = (y * width + x) * channels;
	for (int i = 0; i < channels; ++i) {
		pixel.push_back(static_cast<float>(data[index + i]) / 255.0f);
	}

	return pixel;
}

FileWizard::Image *FileWizard::readImage(const char *filename) {
	auto *image = new Image{};
	image->data = stbi_load(filename, &image->width, &image->height, &image->channels, 0);
	if (!image->data) {
		std::cerr << "Picture couldn't be loaded" << std::endl;
	}
	return image;
}


char* FileWizard::readFile(const char* fileName)
{
	std::ifstream file;

	try
	{
		file.open(fileName);
		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();
		std::string content = buffer.str();

		const size_t length = content.size() + 1;
		auto result = new char[length];
		strcpy(result, content.c_str());
		return result;
	}
	catch ([[maybe_unused]] std::ifstream::failure &e) {
		std::cerr << "Failed: Read File " << fileName << std::endl;
		file.close();
		auto empty = new char[1];
		empty[0] = '\0';
		return empty;
	}
}

