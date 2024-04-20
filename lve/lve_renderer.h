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
		VkRenderPass GetSwapchainRenderPass() const { return m_swapchain->GetRenderPass(); }
		F32 GetAspectRatio() const { return m_swapchain->GetExtentAspectRatio(); }
		bool IsFrameInProgress() const { return m_isFrameStarted; }

		VkCommandBuffer GetCurrentCommandBuffer() const
		{
			ASSERT(IsFrameInProgress(), "Could not get command buffer when frame is not in progress!");
			return m_commandBuffers[m_currentFrameIndex];
		}

		U32 GetCurrentFrameIndex() const
		{
			ASSERT(IsFrameInProgress(), "Could not get current frame index when frame is not in progress!");
			return m_currentFrameIndex;
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
		LveWindow& m_window;
		LveDevice& m_device;

		UniqueRef<LveSwapchain> m_swapchain;
		std::vector<VkCommandBuffer> m_commandBuffers;

		U32 m_currentImageIndex = 0;
		U32 m_currentFrameIndex = 0;
		bool m_isFrameStarted = false;
	};

}  // namespace lve
