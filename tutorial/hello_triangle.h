//
// Created by Junhao Wang (@forkercat) on 3/9/24.
//

#pragma once

#include "core/core.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class HelloTriangleApplication
{
public:
	void Run();

private:
	void InitWindow();

	void InitVulkan();
	void CreateInstance();
	void SetupDebugMessenger();

	void MainLoop();
	void Cleanup();

	static std::vector<const char*> GetRequiredExtensions();
	static bool CheckValidationLayerSupport();

private:
	GLFWwindow* mWindow;
	VkInstance mInstance;
	VkDebugUtilsMessengerEXT mDebugMessenger;
};
