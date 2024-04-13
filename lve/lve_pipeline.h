//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#pragma once

#include "core/core.h"
#include "lve_device.h"

#include <vector>

namespace lve {

	struct PipelineConfigInfo
	{
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		U32 subpass = 0;
	};

	class LvePipeline
	{
	public:
		LvePipeline(LveDevice& device, const std::string& vertFilepath, const std::string& fragFilepath,
					const PipelineConfigInfo& configInfo);
		~LvePipeline();

		LvePipeline(const LvePipeline&) = delete;
		void operator=(const LvePipeline&) = delete;

		void Bind(VkCommandBuffer commandBuffer);

		static PipelineConfigInfo DefaultPipelineConfigInfo(U32 width, U32 height);

	private:
		void CreateGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath,
									const PipelineConfigInfo& configInfo);

		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* pShaderModule);

		static std::vector<char> ReadFile(const std::string& filepath);

	private:
		LveDevice& mDeviceRef;
		VkPipeline mGraphicsPipeline;
		VkShaderModule mVertShaderModule;
		VkShaderModule mFragShaderModule;
	};

}  // namespace lve
