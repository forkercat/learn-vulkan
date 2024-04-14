//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "first_app.h"

#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {

	FirstApp::FirstApp()
	{
		LoadGameObjects();
	}

	FirstApp::~FirstApp()
	{
	}

	void FirstApp::Run()
	{
		SimpleRenderSystem simpleRenderSystem(mDevice, mRenderer.GetSwapchainRenderPass());

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

				simpleRenderSystem.RenderGameObjects(commandBuffer, mGameObjects);
				
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

}  // namespace lve
