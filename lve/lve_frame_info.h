//
// Created by Junhao Wang (@forkercat) on 4/22/24.
//

#pragma once

#include "lve_camera.h"
#include "lve_game_object.h"

#include <vulkan/vulkan.h>

namespace lve
{
	struct FrameInfo
	{
		U32 frameIndex;
		F32 frameTime;
		VkCommandBuffer commandBuffer;
		VkDescriptorSet globalDescriptorSet;
		LveCamera& camera;
		LveGameObject::Map& gameObjects;
	};

} // namespace lve
