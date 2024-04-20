//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "first_app.h"

#include "lve/system/simple_render_system.h"
#include "lve/system/rainbow_system.h"

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

		LveCamera camera;
		// camera.SetViewDirection(Vector3(0.0f), Vector3(0.5f, 0.0f, 1.0f));
		camera.SetViewTarget(Vector3(-1.0f, -2.0f, 2.0f), Vector3(0.0f, 0.0f, 2.5f));

		while (!m_window.ShouldClose())
		{
			// Update time.
			F64 newTime = glfwGetTime();
			F32 deltaTime = static_cast<F32>(newTime - m_elapsedTime);
			m_elapsedTime = newTime;

			glfwPollEvents();

			F32 aspect = m_renderer.GetAspectRatio();
			// camera.SetOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
			camera.SetPerspectiveProjection(MathOp::Radians(50.f), aspect, 0.1f, 10.f);

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

				simpleRenderSystem.RenderGameObjects(commandBuffer, m_gameObjects, camera);

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
		cube.transform.translation = { 0.f, 0.f, 2.5f };
		cube.transform.scale = Vector3(0.5f);

		m_gameObjects.push_back(std::move(cube));
	}

}  // namespace lve
