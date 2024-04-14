//
// Created by Junhao Wang (@forkercat) on 4/14/24.
//

#pragma once

#include "core/core.h"

#include "lve_window.h"
#include "lve_device.h"
#include "lve_swapchain.h"
#include "lve_model.h"

#include <vector>
#include <memory>

namespace lve {

	// Renderer class that manages swapchain and command buffers.
	class LveRenderer
	{
	public:
		LveRenderer(LveWindow& window, LveDevice& device);
		~LveRenderer();

		LveRenderer(const LveRenderer&) = delete;
		LveRenderer& operator=(const LveRenderer&) = delete;

		// Public getter.
		VkRenderPass GetSwapchainRenderPass() const { return mSwapchain->GetRenderPass(); }
		bool IsFrameInProgress() const { return mIsFrameStarted; }

		VkCommandBuffer GetCurrentCommandBuffer() const
		{
			ASSERT(IsFrameInProgress(), "Could not get command buffer when frame is not in progress!");
			return mCommandBuffers[mCurrentImageIndex];
		}

		// Functions to render.
		VkCommandBuffer BeginFrame();
		void EndFrame();
		void BeginSwapchainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapchainRenderPass(VkCommandBuffer commandBuffer);

	private:
		// Functions to create Vulkan resources.
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void RecreateSwapchain();

	private:
		LveWindow& mWindow;
		LveDevice& mDevice;

		std::unique_ptr<LveSwapchain> mSwapchain;
		std::vector<VkCommandBuffer> mCommandBuffers;

		U32 mCurrentImageIndex = 0;
		bool mIsFrameStarted = false;
	};

}  // namespace lve
