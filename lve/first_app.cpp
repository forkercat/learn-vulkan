//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "first_app.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {

	struct SimplePushConstantData
	{
		glm::mat2 transform{ 1.0f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	FirstApp::FirstApp()
	{
		LoadGameObjects();
		CreatePipelineLayout();
		CreatePipeline();
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

			// Could be nullptr if, for example, the swapchain needs to be recreated.
			if (VkCommandBuffer commandBuffer = mRenderer.BeginFrame())
			{
				// The reason why BeginFrame and BeginSwapchainRenderPass are separate functions is
				// we want the app to control over this to enable us easily integrating multiple render passes.
				// - Begin offscreen shadow pass
				// - Render shadow casting objects
				// - End offscreen shadow pass
				// - Post processing...

				mRenderer.BeginSwapchainRenderPass(commandBuffer);
				RenderGameObjects(commandBuffer);
				mRenderer.EndSwapchainRenderPass(commandBuffer);
				mRenderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(mDevice.GetDevice());
	}

	void FirstApp::LoadGameObjects()
	{
		std::vector<LveModel::Vertex> vertices{ { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
												{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
												{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } } };

		auto model = std::make_shared<LveModel>(mDevice, vertices);

		LveGameObject triangle = LveGameObject::CreateGameObject();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };

		triangle.transform2d.translation.x = 0.2f;
		triangle.transform2d.scale = { 2.0f, 0.5f };
		triangle.transform2d.rotation = 0.25f * glm::two_pi<F32>();

		mGameObjects.push_back(std::move(triangle));
	}

	void FirstApp::CreatePipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		// This will be referenced throughout the program's lifetime.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		VkResult result = vkCreatePipelineLayout(mDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create pipeline layout!");
	}

	void FirstApp::CreatePipeline()
	{
		ASSERT(mPipelineLayout, "Could not create pipeline before pipeline layout!");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = mRenderer.GetSwapchainRenderPass();
		pipelineConfig.pipelineLayout = mPipelineLayout;
		mPipeline =
			std::make_unique<LvePipeline>(mDevice, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
	}

	void FirstApp::RenderGameObjects(VkCommandBuffer commandBuffer)
	{
		// Update objects.
		int i = 0;
		for (LveGameObject& gameObject : mGameObjects)
		{
			i += 1;
			gameObject.transform2d.rotation = glm::mod<F32>(gameObject.transform2d.rotation + 0.001f * i, 2.0f * glm::pi<F32>());
		}

		// Bind graphics pipeline.
		mPipeline->Bind(commandBuffer);

		// Render objects.
		for (LveGameObject& gameObject : mGameObjects)
		{
			SimplePushConstantData push{};
			push.offset = gameObject.transform2d.translation;
			push.color = gameObject.color;
			push.transform = gameObject.transform2d.GetTransform();

			vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
							   sizeof(SimplePushConstantData), &push);

			gameObject.model->Bind(commandBuffer);
			gameObject.model->Draw(commandBuffer);
		}
	}

}  // namespace lve
