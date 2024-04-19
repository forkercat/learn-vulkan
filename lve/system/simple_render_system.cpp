//
// Created by Junhao Wang (@forkercat) on 4/14/24.
//

#include "simple_render_system.h"

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

	SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass)
		: m_device(device)
	{
		CreatePipelineLayout();
		CreatePipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(m_device.GetDevice(), m_pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::CreatePipelineLayout()
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

		VkResult result = vkCreatePipelineLayout(m_device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create pipeline layout!");
	}

	void SimpleRenderSystem::CreatePipeline(VkRenderPass renderPass)
	{
		ASSERT(m_pipelineLayout, "Could not create pipeline before pipeline layout!");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_pipeline =
			MakeUniqueRef<LvePipeline>(m_device, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
	}

	void SimpleRenderSystem::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects)
	{
		// Update objects.
		int i = 0;
		for (LveGameObject& gameObject : gameObjects)
		{
			i += 1;
			gameObject.transform2d.rotation = glm::mod<F32>(gameObject.transform2d.rotation + 0.001f * i, 2.0f * glm::pi<F32>());
		}

		// Bind graphics pipeline.
		m_pipeline->Bind(commandBuffer);

		// Render objects.
		for (LveGameObject& gameObject : gameObjects)
		{
			SimplePushConstantData push{};
			push.offset = gameObject.transform2d.translation;
			push.color = gameObject.color;
			push.transform = gameObject.transform2d.GetTransform();

			vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
							   sizeof(SimplePushConstantData), &push);

			gameObject.model->Bind(commandBuffer);
			gameObject.model->Draw(commandBuffer);
		}
	}

}  // namespace lve
