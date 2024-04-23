//
// Created by Junhao Wang (@forkercat) on 4/14/24.
//

#pragma once

#include "core/core.h"

#include "lve/lve_camera.h"
#include "lve/lve_device.h"
#include "lve/lve_pipeline.h"
#include "lve/lve_game_object.h"
#include "lve/lve_frame_info.h"

#include <vector>
#include <memory>

namespace lve
{
	class PointLightSystem
	{
	public:
		PointLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void Render(FrameInfo& frameInfo);

	private:
		void CreatePipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
		void CreatePipeline(VkRenderPass renderPass);

	private:
		LveDevice& m_device;

		UniqueRef<LvePipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout;
	};

} // namespace lve
