//
// Created by Junhao Wang (@forkercat) on 4/3/24.
//

#pragma once

#include "core/core.h"

#include "lve_device.h"

#include <memory>
#include <vector>

namespace lve {

	// Swapchain class that manages Vulkan swapchain and images, framebuffers, render passes
	// synchronization primitives, etc.
	class LveSwapchain
	{
	public:
		static constexpr int MaxFramesInFlight = 2;

		LveSwapchain(LveDevice& device, VkExtent2D windowExtent);
		LveSwapchain(LveDevice& device, VkExtent2D windowExtent, std::shared_ptr<LveSwapchain> previous);
		~LveSwapchain();

		LveSwapchain(const LveSwapchain&) = delete;
		void operator=(const LveSwapchain&) = delete;

		// Functions to get Vulkan resources
		VkFramebuffer GetFramebuffer(int index) { return mSwapchainFramebuffers[index]; }
		VkRenderPass GetRenderPass() { return mRenderPass; }
		VkImageView GetImageView(int index) { return mSwapchainImageViews[index]; }

		// Functions to get swapchain info
		USize GetImageCount() { return mSwapchainImages.size(); }
		VkFormat GetSwapchainImageFormat() { return mSwapchainImageFormat; }
		VkExtent2D GetSwapchainExtent() { return mSwapchainExtent; }
		U32 GetWidth() { return mSwapchainExtent.width; }
		U32 GetHeight() { return mSwapchainExtent.height; }
		F32 GetExtentAspectRatio() { return static_cast<F32>(mSwapchainExtent.width) / static_cast<F32>(mSwapchainExtent.height); }

		// Public functions
		VkFormat FindDepthFormat();
		VkResult AcquireNextImage(U32* imageIndex);
		VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, U32* imageIndex);

		bool CompareSwapchainFormats(const LveSwapchain& otherSwapchain) const
		{
			return mSwapchainImageFormat == otherSwapchain.mSwapchainImageFormat &&
				   mSwapchainDepthFormat == otherSwapchain.mSwapchainDepthFormat;
		}

	private:
		// Functions to create Vulkan resources
		void Init();
		void CreateSwapchain();
		void CreateImageViews();
		void CreateDepthResources();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	private:
		LveDevice& mDevice;
		VkExtent2D mWindowExtent;
		VkSwapchainKHR mSwapchain;
		std::shared_ptr<LveSwapchain> mOldSwapchain;

		VkFormat mSwapchainImageFormat;
		VkFormat mSwapchainDepthFormat;
		VkExtent2D mSwapchainExtent;
		VkRenderPass mRenderPass;

		// Images
		std::vector<VkFramebuffer> mSwapchainFramebuffers;
		std::vector<VkImage> mSwapchainImages;
		std::vector<VkImageView> mSwapchainImageViews;

		std::vector<VkImage> mDepthImages;
		std::vector<VkDeviceMemory> mDepthImageMemorys;
		std::vector<VkImageView> mDepthImageViews;

		// Sync
		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;  // size = 2
		std::vector<VkFence> mImagesInFlight;  // size = 3

		USize mCurrentFrame = 0;
	};

}  // namespace lve
