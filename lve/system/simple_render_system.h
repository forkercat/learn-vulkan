//
// Created by Junhao Wang (@forkercat) on 4/14/24.
//

#pragma once

#include "core/core.h"

#include "lve/lve_camera.h"
#include "lve/lve_device.h"
#include "lve/lve_pipeline.h"
#include "lve/lve_game_object.h"

#include <vector>
#include <memory>

namespace lve
{
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects, const LveCamera& camera);

	private:
		void CreatePipelineLayout();
		void CreatePipeline(VkRenderPass renderPass);

	private:
		LveDevice& m_device;

		UniqueRef<LvePipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout;
	};

} // namespace lve
