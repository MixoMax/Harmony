#pragma once
#ifndef FILEWIZARD_H
#define FILEWIZARD_H

#include <vector>
#include "stb_image.h"

class FileWizard
{
public:
	struct Image {
		int width, height;
		int channels;
		unsigned char *data;
		~Image() {
			stbi_image_free(data);
		}

		[[nodiscard]] std::vector<float> getPixel(int x, int y) const;
	};
	static Image *readImage(const char *filename);

	static char* readFile(const char* fileName);
};

#endif

