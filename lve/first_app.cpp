//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "first_app.h"

#include "lve/system/simple_render_system.h"
#include "lve/system/rainbow_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {

	FirstApp::FirstApp()
		: m_elapsedTime(0.0f)
	{
		LoadGameObjects();
	}

	FirstApp::~FirstApp()
	{
	}

	void FirstApp::Run()
	{
		SimpleRenderSystem simpleRenderSystem(m_device, m_renderer.GetSwapchainRenderPass());
		RainbowSystem rainbowSystem(0.4f);

		while (!m_window.ShouldClose())
		{
			// Update time.
			F64 newTime = glfwGetTime();
			F32 deltaTime = static_cast<F32>(newTime - m_elapsedTime);
			m_elapsedTime = newTime;

			glfwPollEvents();

			// Could be nullptr if, for example, the swapchain needs to be recreated.
			if (VkCommandBuffer commandBuffer = m_renderer.BeginFrame())
			{
				// The reason why BeginFrame and BeginSwapchainRenderPass are separate functions is
				// we want the app to control over this to enable us easily integrating multiple render passes.
				//
				// - BeginFrame to acquire image and begin command buffer
				// - Begin offscreen shadow pass
				// -   Render shadow casting objects
				// - End offscreen shadow pass
				// - Begin shading pass
				// -   Render objects
				// - End shading pass
				// - Post processing...
				m_renderer.BeginSwapchainRenderPass(commandBuffer);

				rainbowSystem.Update(deltaTime, m_gameObjects);

				simpleRenderSystem.RenderGameObjects(commandBuffer, m_gameObjects);

				m_renderer.EndSwapchainRenderPass(commandBuffer);
				m_renderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(m_device.GetDevice());
	}

	void FirstApp::LoadGameObjects()
	{
		UniqueRef<LveModel> cubeModel = LveModel::CreateCubeModel(m_device, { 0.f, 0.f, 0.f });

		LveGameObject cube = LveGameObject::CreateGameObject();
		cube.model = std::move(cubeModel);
		cube.transform.translation = { 0.f, 0.f, 0.5f };
		cube.transform.scale = glm::vec3(0.5f);

		m_gameObjects.push_back(std::move(cube));
	}

}  // namespace lve
