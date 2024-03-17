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

	// Instance, device & validation layers
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateWindowSurface();
	void PickPhysicalDevice();
	void CreateLogicalDeviceAndQueues();

	// Swapchain & pipeline
	void RecreateSwapchain();
	void CleanUpSwapchain();

	void CreateSwapchain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateFramebuffers();

	// Command buffer
	void CreateCommandPool();
	void CreateCommandBuffers();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, U32 imageIndex);

	// Synchronization
	void CreateSyncObjects();

	// Draw
	void DrawFrame();

	void MainLoop();
	void CleanUp();

	static std::vector<const char*> GetRequiredInstanceExtensions();
	static bool CheckValidationLayerSupport();

	bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D ChooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

	static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

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
	VkSwapchainKHR mSwapchain;
	VkFormat mSwapchainImageFormat;
	VkExtent2D mSwapchainExtent;
	std::vector<VkImage> mSwapchainImages;	// Auto destroyed when the swap chain is cleaned up.
	std::vector<VkImageView> mSwapchainImageViews;
	std::vector<VkFramebuffer> mSwapchainFramebuffers;

	// Graphics pipeline
	VkRenderPass mRenderPass;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;

	// Commands
	VkCommandPool mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;

	// Synchronization
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mInFlightFences;

	// Keep track of the current frame to use the right sync objects. Having multiple frames in flight enables us start
	// rendering the next, with rendering of one frame to not interfere with the recording of the next.
	// Why 2 frames? Don't want the CPU to get too far ahead of the GPU.
	// The CPU and the GPU can be working on their own tasks at the same time.
	// -----------------------------------------------------------------------
	// Records 1st frame on CPU -> Renders 1st frame on GPU -> Renders 2nd frame on GPU
	//                          -> Records 2nd frame on CPU -> Records 1st frame on CPU
	U32 mCurrentFrame = 0;
	static const U32 kMaxFramesInFlight = 2;

	// Handling resizes explicitly for drivers/platforms that cannot trigger VK_ERROR_OUT_OF_DATE_KHR.
	bool mFramebufferResized = false;
};
