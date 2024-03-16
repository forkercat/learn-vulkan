//
// Created by Junhao Wang (@forkercat) on 3/15/24.
//

#pragma once

#include "core/core.h"

#include <vulkan/vulkan.h>

#include <iostream>
#include <fstream>

class Shader
{
public:
	static std::vector<char> ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate || std::ios::binary);

		ASSERT(file.is_open(), "Failed to open shader file!");

		USize fileSize = static_cast<USize>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		PRINT("Loaded shader %s (%u bytes)", filename.c_str(), fileSize);

		file.close();

		return buffer;
	}
};
