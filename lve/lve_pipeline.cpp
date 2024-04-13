//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "lve_pipeline.h"

#include <fstream>

namespace lve {

	LvePipeline::LvePipeline(LveDevice& device, const std::string& vertFilepath, const std::string& fragFilepath,
							 const PipelineConfigInfo& configInfo)
		: mDeviceRef(device)
	{
		CreateGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
	}

	LvePipeline::~LvePipeline()
	{
		if (mFragShaderModule != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(mDeviceRef.GetDevice(), mFragShaderModule, nullptr);
		}

		if (mVertShaderModule != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(mDeviceRef.GetDevice(), mVertShaderModule, nullptr);
		}

		vkDestroyPipeline(mDeviceRef.GetDevice(), mGraphicsPipeline, nullptr);
	}

	void LvePipeline::CreateGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath,
											 const PipelineConfigInfo& configInfo)
	{
		ASSERT(configInfo.pipelineLayout, "Could not create graphics pipeline: No pipeline layout provided!");
		ASSERT(configInfo.renderPass, "Could not create graphics pipeline: No render pass provided!");

		// We can clean up the shader modules after the bytecode is complied to machine code or linked,
		// which happens when the graphics pipeline is created.
		std::vector<char> vertexShaderCode = ReadFile(vertFilepath);
		std::vector<char> fragmentShaderCode = ReadFile(fragFilepath);
		CreateShaderModule(vertexShaderCode, &mVertShaderModule);
		CreateShaderModule(fragmentShaderCode, &mFragShaderModule);

		// Shader stages
		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = mVertShaderModule;
		shaderStages[0].pName = "main";	 // entry point
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;

		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = mFragShaderModule;
		shaderStages[1].pName = "main";	 // entry point
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		// Vertex input info
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		// Viewport state info
		VkPipelineViewportStateCreateInfo viewportInfo{};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = &configInfo.viewport;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = &configInfo.scissor;

		// Graphics pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDynamicState = nullptr;  // Optional
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;

		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
		pipelineInfo.basePipelineIndex = -1;			   // Optional

		VkResult result = vkCreateGraphicsPipelines(mDeviceRef.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create graphics pipeline!");

		// Cleanup shader modules after pipeline creation.
		vkDestroyShaderModule(mDeviceRef.GetDevice(), mFragShaderModule, nullptr);
		vkDestroyShaderModule(mDeviceRef.GetDevice(), mVertShaderModule, nullptr);
		mFragShaderModule = VK_NULL_HANDLE;
		mVertShaderModule = VK_NULL_HANDLE;
	}

	void LvePipeline::Bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
	}

	PipelineConfigInfo LvePipeline::DefaultPipelineConfigInfo(U32 width, U32 height)
	{
		PipelineConfigInfo configInfo{};

		// Input assembly
		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// Viewport and scissor
		// Viewport state info is expected to be empty.
		configInfo.viewport.x = 0.0f;
		configInfo.viewport.y = 0.0f;
		configInfo.viewport.width = static_cast<F32>(width);
		configInfo.viewport.height = static_cast<F32>(height);
		configInfo.viewport.minDepth = 0.0f;
		configInfo.viewport.maxDepth = 1.0f;
		configInfo.scissor.offset = { 0, 0 };
		configInfo.scissor.extent = { width, height };

		// Rasterization
		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		configInfo.rasterizationInfo.lineWidth = 1.0f;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		configInfo.rasterizationInfo.depthBiasClamp = 0.0f;			  // Optional
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;	  // Optional

		// Multisampling
		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.0f;			  // Optional
		configInfo.multisampleInfo.pSampleMask = nullptr;			  // Optional
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;		  // Optional

		// Color blend attachment
		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;	 // Optional
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;	 // Optional
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;				 // Optional
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;	 // Optional
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;	 // Optional
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;				 // Optional

		// Color blend info
		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
		configInfo.colorBlendInfo.blendConstants[0] = 0.0f;	 // Optional
		configInfo.colorBlendInfo.blendConstants[1] = 0.0f;	 // Optional
		configInfo.colorBlendInfo.blendConstants[2] = 0.0f;	 // Optional
		configInfo.colorBlendInfo.blendConstants[3] = 0.0f;	 // Optional

		// Depth and stencil
		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.minDepthBounds = 0.0f;	// Optional
		configInfo.depthStencilInfo.maxDepthBounds = 1.0f;	// Optional
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.front = {};	 // Optional
		configInfo.depthStencilInfo.back = {};	 // Optional

		return configInfo;
	}

	void LvePipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* pShaderModule)
	{
		ASSERT(!code.empty(), "Failed to create a shader module for empty code.");

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		// Alignment: The allocator of std::vector already ensures that the data satisfies
		// the worst case alignment requirements.
		createInfo.pCode = reinterpret_cast<const U32*>(code.data());

		VkResult result = vkCreateShaderModule(mDeviceRef.GetDevice(), &createInfo, nullptr, pShaderModule);
		ASSERT_EQ(result, VK_SUCCESS, "Failed to create shader module!");
	}

	std::vector<char> LvePipeline::ReadFile(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);

		ASSERT(file.is_open(), "Failed to open shader file: %s", filepath.c_str());

		USize fileSize = static_cast<USize>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		PRINT("Loaded shader %s (%zu bytes)", filepath.c_str(), fileSize);

		file.close();
		return buffer;
	}

}  // namespace lve
