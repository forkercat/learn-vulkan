//
// Created by Junhao Wang (@forkercat) on 3/9/24.
//

#pragma once

#include "core/core.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <set>
#include <vector>
#include <limits>
#include <algorithm>

struct QueueFamilyIndices;

class HelloTriangleApplication
{
public:
	void Run();

private:
	void InitWindow();

	void InitVulkan();
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateWindowSurface();
	void PickPhysicalDevice();
	void CreateLogicalDeviceAndQueues();
	void CreateSwapChain();
	void CreateImageViews();

	void MainLoop();
	void CleanUp();

	static std::vector<const char*> GetRequiredInstanceExtensions();
	static bool CheckValidationLayerSupport();

	bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D ChooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

private:
	GLFWwindow* mWindow;
	VkInstance mInstance;
	VkDebugUtilsMessengerEXT mDebugMessenger;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;	// Auto destroyed when VkInstance is destroyed.
	VkDevice mDevice;
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;
	VkSurfaceKHR mSurface;

	// Swap chain
	VkSwapchainKHR mSwapChain;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;
	std::vector<VkImage> mSwapChainImages;	// Auto destroyed when the swap chain is cleaned up.
	std::vector<VkImageView> mSwapChainImageViews;
};
