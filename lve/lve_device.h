//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#pragma once

#include "lve_window.h"

#include <vector>
#include <optional>

namespace lve {

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		std::optional<U32> graphicsFamily;
		std::optional<U32> presentFamily;
		bool IsComplete() { graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	// LveDevice
	class LveDevice
	{
	public:
		LveDevice(LveWindow& window);
		~LveDevice();

		// Make the device not copiable or movable.
		LveDevice(const LveDevice&) = delete;
		void operator=(const LveDevice&) = delete;
		LveDevice(LveDevice&&) = delete;
		LveDevice& operator=(LveDevice&&) = delete;

		// Getter for Vulkan resources
		VkCommandPool GetCommandPool() { return mCommandPool; }
		VkDevice GetDevice() { return mDevice; }
		VkSurfaceKHR GetSurface() { return mSurface; }
		VkQueue GetGraphicsQueue() { return mGraphicsQueue; }
		VkQueue GetPresentQueue() { return mGraphicsQueue; }

		// Public helper functions
		SwapchainSupportDetails GetSwapchainSupport() { return QuerySwapchainSupport(mPhysicalDevice); };
		QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(mPhysicalDevice); }
		U32 FindMemoryType(U32 typeFilter, VkMemoryPropertyFlags propertyFlags);
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& formatCandidates, VkImageTiling tiling,
									 VkFormatFeatureFlags features);

		// Buffer helper functions
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags,
						  VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, U32 width, U32 height, U32 layerCount);

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

		// Image helper functions
		void CreateImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags propertyFlags,
								 VkImage& image, VkDeviceMemory& imageMemory);

	private:
		// Functions to create Vulkan resources
		void CreateInstance();
		void SetUpDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();

		// Private help functions
		bool IsDeviceSuitable(VkPhysicalDevice physicalDevice);
		std::vector<const char*> GetRequiredExtensions();
		bool CheckValidationLayerSupport();
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void HasGlfwRequiredInstanceExtensions();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
		SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physicalDevice);

	public:
		VkPhysicalDeviceProperties properties;

	private:
		VkInstance mInstance;
		VkDebugUtilsMessengerEXT mDebugMessenger;
		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		LveWindow& mWindow;
		VkCommandPool mCommandPool;

		VkDevice mDevice;
		VkSurfaceKHR mSurface;
		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;

#ifdef NDBUG
		const bool mEnableValidationLayers = false;
#else
		const bool mEnableValidationLayers = true;
#endif

		const std::vector<const char*> mValidationLayers{ "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> mDeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME,
														  "VK_KHR_portability_subset" };
	};

}  // namespace lve
