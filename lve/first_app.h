//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#pragma once

#include "lve_window.h"
#include "lve_device.h"
#include "lve_pipeline.h"
#include "lve_swapchain.h"
#include "lve_model.h"
#include "lve_game_object.h"

#include "core/core.h"

#include <vector>

namespace lve {

	class FirstApp
	{
	public:
		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void Run();

	public:
		static constexpr U32 sWidth = 800;
		static constexpr U32 sHeight = 600;

	private:
		void LoadGameObjects();
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void RecordCommandBuffer(int imageIndex);
		void RenderGameObjects(VkCommandBuffer commandBuffer);
		void DrawFrame();

		void RecreateSwapchain();

	private:
		LveWindow mWindow{ sWidth, sHeight, "Hello Vulkan!" };
		LveDevice mDevice{ mWindow };

		std::unique_ptr<LveSwapchain> mSwapchain;
		std::unique_ptr<LvePipeline> mPipeline;

		VkPipelineLayout mPipelineLayout;
		std::vector<VkCommandBuffer> mCommandBuffers;

		std::vector<LveGameObject> mGameObjects;
	};

}  // namespace lve
