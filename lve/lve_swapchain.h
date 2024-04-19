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
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		LveSwapchain(LveDevice& device, VkExtent2D windowExtent);
		LveSwapchain(LveDevice& device, VkExtent2D windowExtent, Ref<LveSwapchain> previous);
		~LveSwapchain();

		LveSwapchain(const LveSwapchain&) = delete;
		void operator=(const LveSwapchain&) = delete;

		// Functions to get Vulkan resources
		VkRenderPass GetRenderPass() { return m_renderPass; }
		VkFramebuffer GetFramebuffer(int index) { return m_swapchainFramebuffers[index]; }
		VkImageView GetImageView(int index) { return m_swapchainImageViews[index]; }

		// Functions to get swapchain info
		USize GetImageCount() { return m_swapchainImages.size(); }
		VkFormat GetSwapchainImageFormat() { return m_swapchainImageFormat; }
		VkExtent2D GetSwapchainExtent() { return m_swapchainExtent; }
		U32 GetWidth() { return m_swapchainExtent.width; }
		U32 GetHeight() { return m_swapchainExtent.height; }
		F32 GetExtentAspectRatio() { return static_cast<F32>(m_swapchainExtent.width) / static_cast<F32>(m_swapchainExtent.height); }

		// Public functions
		VkResult AcquireNextImage(U32* imageIndex);
		VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, U32* imageIndex);
		VkFormat FindDepthFormat();

		bool CompareSwapchainFormats(const LveSwapchain& otherSwapchain) const
		{
			return m_swapchainImageFormat == otherSwapchain.m_swapchainImageFormat &&
				   m_swapchainDepthFormat == otherSwapchain.m_swapchainDepthFormat;
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
		LveDevice& m_device;
		VkExtent2D m_windowExtent;
		VkSwapchainKHR m_swapchain;
		Ref<LveSwapchain> m_oldSwapchain;

		VkFormat m_swapchainImageFormat;
		VkFormat m_swapchainDepthFormat;
		VkExtent2D m_swapchainExtent;
		VkRenderPass m_renderPass;

		// Images
		std::vector<VkFramebuffer> m_swapchainFramebuffers;
		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_swapchainImageViews;

		std::vector<VkImage> m_depthImages;
		std::vector<VkDeviceMemory> m_depthImageMemorys;
		std::vector<VkImageView> m_depthImageViews;

		// Sync
		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;	// size = 2
		std::vector<VkFence> m_imagesInFlight;	// size = 3

		USize m_currentFrame = 0;
	};

}  // namespace lve
