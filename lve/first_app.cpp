//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "first_app.h"

namespace lve {

	FirstApp::FirstApp()
	{
		CreatePipelineLayout();
		CreatePipeline();
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
		PipelineConfigInfo pipelineConfig =
			LvePipeline::DefaultPipelineConfigInfo(mSwapchain.GetSwapchainExtent().width, mSwapchain.GetSwapchainExtent().height);

		pipelineConfig.renderPass = mSwapchain.GetRenderPass();
		pipelineConfig.pipelineLayout = mPipelineLayout;
		mPipeline =
			std::make_unique<LvePipeline>(mDevice, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
	}

	void FirstApp::CreateCommandBuffers()
	{
		mCommandBuffers.resize(mSwapchain.GetImageCount());

		VkCommandBufferAllocateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferInfo.commandPool = mDevice.GetCommandPool();
		bufferInfo.commandBufferCount = static_cast<U32>(mCommandBuffers.size());

		VkResult result = vkAllocateCommandBuffers(mDevice.GetDevice(), &bufferInfo, mCommandBuffers.data());
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create command buffers!");

		for (int i = 0; i < mCommandBuffers.size(); i++)
		{
			// Begin command buffer.
			VkCommandBufferBeginInfo bufferBeginInfo{};
			bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			VkResult beginResult = vkBeginCommandBuffer(mCommandBuffers[i], &bufferBeginInfo);
			ASSERT_EQ(beginResult, VK_SUCCESS, "Failed to begin recording command buffer!");

			// Begin render pass.
			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = mSwapchain.GetRenderPass();
			renderPassBeginInfo.framebuffer = mSwapchain.GetFramebuffer(i);

			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = mSwapchain.GetSwapchainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { { 0.01f, 0.01f, 0.01f, 1.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassBeginInfo.clearValueCount = static_cast<U32>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Bind graphics pipeline.
			mPipeline->Bind(mCommandBuffers[i]);
			vkCmdDraw(mCommandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(mCommandBuffers[i]);
			VkResult endResult = vkEndCommandBuffer(mCommandBuffers[i]);
			ASSERT_EQ(endResult, VK_SUCCESS, "Failed to end command buffer!");
		}
	}

	void FirstApp::DrawFrame()
	{
		U32 imageIndex{};
		VkResult acquireResult = mSwapchain.AcquireNextImage(&imageIndex);
		ASSERT(acquireResult == VK_SUCCESS || acquireResult == VK_SUBOPTIMAL_KHR, "Failed to acquire next image!");

		VkResult submitResult = mSwapchain.SubmitCommandBuffers(&mCommandBuffers[imageIndex], &imageIndex);
		ASSERT_EQ(submitResult, VK_SUCCESS, "Failed to submit command buffer!");
	}

}  // namespace lve
