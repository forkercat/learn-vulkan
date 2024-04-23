//
// Created by Junhao Wang (@forkercat) on 4/22/24.
//

#pragma once

#include "lve_camera.h"

#include <vulkan/vulkan.h>

namespace lve
{
	struct FrameInfo
	{
		U32 frameIndex;
		F32 frameTime;
		VkCommandBuffer commandBuffer;
		LveCamera& camera;
		VkDescriptorSet globalDescriptorSet;
	};

} // namespace lve
