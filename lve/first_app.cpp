//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "first_app.h"

namespace lve {

	FirstApp::FirstApp()
	{
		LoadModels();
		CreatePipelineLayout();
		RecreateSwapchain();  // also create the pipeline

		// For now, the command buffers are created once and will be reused in frames.
		CreateCommandBuffers();
	}

	FirstApp::~FirstApp()
	{
		vkDestroyPipelineLayout(mDevice.GetDevice(), mPipelineLayout, nullptr);
	}

	void FirstApp::Run()
	{
		while (!mWindow.ShouldClose())
		{
			glfwPollEvents();
			DrawFrame();
		}

		vkDeviceWaitIdle(mDevice.GetDevice());
	}

	void FirstApp::LoadModels()
	{
		std::vector<LveModel::Vertex> vertices{ { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
												{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
												{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } } };
		mModel = std::make_unique<LveModel>(mDevice, vertices);
	}

	void FirstApp::CreatePipelineLayout()
	{
		// This will be referenced throughout the program's lifetime.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VkResult result = vkCreatePipelineLayout(mDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create pipeline layout!");
	}

	void FirstApp::CreatePipeline()
	{
		ASSERT(mSwapchain, "Could not create pipeline before swapchain!");
		ASSERT(mPipelineLayout, "Could not create pipeline before pipeline layout!");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = mSwapchain->GetRenderPass();
		pipelineConfig.pipelineLayout = mPipelineLayout;
		mPipeline =
			std::make_unique<LvePipeline>(mDevice, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
	}

	void FirstApp::CreateCommandBuffers()
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

	void FirstApp::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(mDevice.GetDevice(), mDevice.GetCommandPool(), static_cast<U32>(mCommandBuffers.size()), mCommandBuffers.data());
		mCommandBuffers.clear();
	}

	void FirstApp::RecordCommandBuffer(int imageIndex)
	{
		// Begin command buffer.
		VkCommandBufferBeginInfo bufferBeginInfo{};
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkResult beginResult = vkBeginCommandBuffer(mCommandBuffers[imageIndex], &bufferBeginInfo);
		ASSERT_EQ(beginResult, VK_SUCCESS, "Failed to begin recording command buffer!");

		// Begin render pass.
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mSwapchain->GetRenderPass();
		renderPassBeginInfo.framebuffer = mSwapchain->GetFramebuffer(imageIndex);

		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = mSwapchain->GetSwapchainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { { 0.01f, 0.01f, 0.01f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassBeginInfo.clearValueCount = static_cast<U32>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(mCommandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Dynamic viewport and scissor
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<F32>(mSwapchain->GetSwapchainExtent().width);
		viewport.height = static_cast<F32>(mSwapchain->GetSwapchainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mCommandBuffers[imageIndex], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = mSwapchain->GetSwapchainExtent();
		vkCmdSetScissor(mCommandBuffers[imageIndex], 0, 1, &scissor);

		// Bind graphics pipeline.
		mPipeline->Bind(mCommandBuffers[imageIndex]);

		// Draw
		mModel->Bind(mCommandBuffers[imageIndex]);
		mModel->Draw(mCommandBuffers[imageIndex]);

		vkCmdEndRenderPass(mCommandBuffers[imageIndex]);
		VkResult endResult = vkEndCommandBuffer(mCommandBuffers[imageIndex]);
		ASSERT_EQ(endResult, VK_SUCCESS, "Failed to end command buffer!");
	}

	void FirstApp::DrawFrame()
	{
		// Needs to synchronize the below calls because on GPU they are executed asynchronously.
		// 1. Acquire an image from the swapchain.
		// 2. Execute commands that draw onto the acquired image.
		// 3. Present that image to the screen for presentation, returning it to the swapchain.
		// The function calls will return before the operations are actually finished and the order of execution is also undefined.

		U32 imageIndex{};
		VkResult acquireResult = mSwapchain->AcquireNextImage(&imageIndex);

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapchain();
			return;
		}

		if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
		{
			ASSERT(false, "Failed to acquire next image!");
		}

		RecordCommandBuffer(imageIndex);

		VkResult submitResult = mSwapchain->SubmitCommandBuffers(&mCommandBuffers[imageIndex], &imageIndex);

		if (submitResult == VK_ERROR_OUT_OF_DATE_KHR || submitResult == VK_SUBOPTIMAL_KHR || mWindow.WasWindowResized())
		{
			mWindow.ResetWindowResizedFlag();
			RecreateSwapchain();
			return;
		}

		ASSERT_EQ(submitResult, VK_SUCCESS, "Failed to submit command buffer!");
	}

	void FirstApp::RecreateSwapchain()
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
			mSwapchain = std::make_unique<LveSwapchain>(mDevice, extent, std::move(mSwapchain));

			if (mSwapchain->GetImageCount() != mCommandBuffers.size())
			{
				FreeCommandBuffers();
				CreateCommandBuffers();
			}
		}

		CreatePipeline();
	}

}  // namespace lve
