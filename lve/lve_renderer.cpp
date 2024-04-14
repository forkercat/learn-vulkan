//
// Created by Junhao Wang (@forkercat) on 4/14/24.
//

#include "lve_renderer.h"

namespace lve {

	LveRenderer::LveRenderer(LveWindow& window, LveDevice& device)
		: mWindow(window), mDevice(device)
	{
		RecreateSwapchain();  // also create the pipeline

		// For now, the command buffers are created once and will be reused in frames.
		CreateCommandBuffers();
	}

	LveRenderer::~LveRenderer()
	{
		FreeCommandBuffers();
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Functions to render.
	/////////////////////////////////////////////////////////////////////////////////

	VkCommandBuffer LveRenderer::BeginFrame()
	{
		ASSERT(!mIsFrameStarted, "Could not call BeginFrame while already in frame progress!");

		// Needs to synchronize the below calls because on GPU they are executed asynchronously.
		// 1. Acquire an image from the swapchain.
		// 2. Execute commands that draw onto the acquired image.
		// 3. Present that image to the screen for presentation, returning it to the swapchain.
		// The function calls will return before the operations are actually finished and the order of execution is also undefined.

		VkResult acquireResult = mSwapchain->AcquireNextImage(&mCurrentImageIndex);

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapchain();
			return nullptr;
		}

		if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
		{
			ASSERT(false, "Failed to acquire next image!");
		}

		mIsFrameStarted = true;

		VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

		// Begin command buffer.
		VkCommandBufferBeginInfo bufferBeginInfo{};
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkResult beginResult = vkBeginCommandBuffer(commandBuffer, &bufferBeginInfo);
		ASSERT_EQ(beginResult, VK_SUCCESS, "Failed to begin recording command buffer!");

		return commandBuffer;
	}

	void LveRenderer::EndFrame()
	{
		ASSERT(mIsFrameStarted, "Could not call EndFrame while frame is not in progress!");

		VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

		VkResult endResult = vkEndCommandBuffer(commandBuffer);
		ASSERT_EQ(endResult, VK_SUCCESS, "Failed to end command buffer!");

		// Submit command buffer.
		VkResult submitResult = mSwapchain->SubmitCommandBuffers(&commandBuffer, &mCurrentImageIndex);

		if (submitResult == VK_ERROR_OUT_OF_DATE_KHR || submitResult == VK_SUBOPTIMAL_KHR || mWindow.WasWindowResized())
		{
			mWindow.ResetWindowResizedFlag();
			RecreateSwapchain();
		}
		else if (submitResult != VK_SUCCESS)
		{
			ASSERT(false, "Failed to submit command buffer!");
		}

		mIsFrameStarted = false;
	}

	void LveRenderer::BeginSwapchainRenderPass(VkCommandBuffer commandBuffer)
	{
		ASSERT(mIsFrameStarted, "Could not begin render pass while frame is not in progress!");
		ASSERT(commandBuffer == GetCurrentCommandBuffer(), "Could not begin render pass on command buffer from a different frame!");

		// Begin render pass.
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mSwapchain->GetRenderPass();
		renderPassBeginInfo.framebuffer = mSwapchain->GetFramebuffer(mCurrentImageIndex);

		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = mSwapchain->GetSwapchainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { { 0.01f, 0.01f, 0.01f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassBeginInfo.clearValueCount = static_cast<U32>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Dynamic viewport and scissor
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<F32>(mSwapchain->GetSwapchainExtent().width);
		viewport.height = static_cast<F32>(mSwapchain->GetSwapchainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = mSwapchain->GetSwapchainExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void LveRenderer::EndSwapchainRenderPass(VkCommandBuffer commandBuffer)
	{
		ASSERT(mIsFrameStarted, "Could not end render pass while frame is not in progress!");
		ASSERT(commandBuffer == GetCurrentCommandBuffer(), "Could not end render pass on command buffer from a different frame!");

		vkCmdEndRenderPass(commandBuffer);
	}

	/////////////////////////////////////////////////////////////////////////////////
	// Functions to create Vulkan resources.
	/////////////////////////////////////////////////////////////////////////////////

	void LveRenderer::CreateCommandBuffers()
	{
		mCommandBuffers.resize(mSwapchain->GetImageCount());

		VkCommandBufferAllocateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferInfo.commandPool = mDevice.GetCommandPool();
		bufferInfo.commandBufferCount = static_cast<U32>(mCommandBuffers.size());

		VkResult result = vkAllocateCommandBuffers(mDevice.GetDevice(), &bufferInfo, mCommandBuffers.data());
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create command buffers!");
	}

	void LveRenderer::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(mDevice.GetDevice(), mDevice.GetCommandPool(), static_cast<U32>(mCommandBuffers.size()),
							 mCommandBuffers.data());
		mCommandBuffers.clear();
	}

	void LveRenderer::RecreateSwapchain()
	{
		VkExtent2D extent = mWindow.GetExtent();

		// Handles window minimization.
		while (extent.width == 0 || extent.height == 0)
		{
			extent = mWindow.GetExtent();
			glfwWaitEvents();
		}

		// Need to wait for the current swapchain not being used.
		vkDeviceWaitIdle(mDevice.GetDevice());

		if (mSwapchain == nullptr)
		{
			mSwapchain = std::make_unique<LveSwapchain>(mDevice, extent);
		}
		else
		{
			std::shared_ptr oldSwapchain = std::move(mSwapchain);
			mSwapchain = std::make_unique<LveSwapchain>(mDevice, extent, oldSwapchain);

			// Since we are not recreating the pipeline, we need to check if the swapchain render pass
			// is still compatible with the color or depth format defined in the pipeline render pass.
			if (!oldSwapchain->CompareSwapchainFormats(*mSwapchain.get()))
			{
				ASSERT(false, "Failed to recreate swapchain. The swapchain image or depth format has changed!");
			}

			if (mSwapchain->GetImageCount() != mCommandBuffers.size())
			{
				FreeCommandBuffers();
				CreateCommandBuffers();
			}
		}
	}

}  // namespace lve
