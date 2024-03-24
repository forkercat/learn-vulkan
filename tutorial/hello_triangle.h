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
	// Initialize GLFW window.
	void InitWindow();
	// Initialize Vulkan resources.
	void InitVulkan();

	/// Instance, device & validation layers

	// Create a Vulkan instance. Usually you would create the Vulkan application and check and specify required
	// extensions by GLFW, your platform, and validation layer.
	void CreateInstance();
	// Set up the debug messenger callback for validation layer. You would also configure the validation debug options.
	void SetupDebugMessenger();
	// Create window surface. Since GLFW supports Vulkan API, we will use GLFW API to create the Vulkan surface object.
	void CreateWindowSurface();
	// Pick an available physical device and check if required device extensions are supported (e.g. swapchain).
	// We can also query the physical device about queue families and device features.
	void PickPhysicalDevice();
	// Create a logical device and queues.
	void CreateLogicalDeviceAndQueues();

	/// Swapchain & pipeline
	void RecreateSwapchain();
	void CleanUpSwapchain();

	// Create a swapchain with swap images and queues.
	void CreateSwapchain();
	void CreateImageViews();
	// Create a render pass with subpasses that define attachment formats.
	void CreateRenderPass();
	// Create a graphics pipeline state object (PSO) with shaders. Set the pipeline's render pass.
	void CreateGraphicsPipeline();
	// Create framebuffers for swap image views. Note that we create framebuffers after and later when we begin the
	// render pass it would require setting framebuffers for the render pass.
	void CreateFramebuffers();

	/// Command buffer

	// Create command pool for a particular family of queues (i.e. graphics).
	void CreateCommandPool();
	void CreateCommandBuffers();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, U32 imageIndex);

	/// Synchronization
	void CreateSyncObjects();

	/// Draw
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
	VkSurfaceKHR mSurface;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;	// Auto destroyed when VkInstance is destroyed.
	VkDevice mDevice;
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;

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
