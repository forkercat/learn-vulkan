//
// Created by Junhao Wang (@forkercat) on 4/1/24.
//

#include "first_app.h"

#include "lve/system/simple_render_system.h"
#include "lve/system/rainbow_system.h"
#include "lve/keyboard_movement_controller.h"

#include <chrono>

namespace lve
{
	struct GlobalUbo
	{
		Matrix4 projectionView{ 1.0f };
		Vector3 lightDirection = MathOp::Normalize(Vector3{ 1.0f, -3.0f, -1.0f });
	};

	FirstApp::FirstApp()
	{
		LoadGameObjects();
	}

	FirstApp::~FirstApp()
	{
	}

	void FirstApp::Run()
	{
		// Uniform buffers
		std::vector<UniqueRef<LveBuffer>> uboBuffers(LveSwapchain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < uboBuffers.size(); ++i)
		{
			uboBuffers[i] = MakeUniqueRef<LveBuffer>(
				m_device,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT); // or add VK_MEMORY_PROPERTY_HOST_COHERENT_BIT

			uboBuffers[i]->Map();
		}

		// Render system, camera, and controller
		SimpleRenderSystem simpleRenderSystem(m_device, m_renderer.GetSwapchainRenderPass());
		RainbowSystem rainbowSystem(0.4f);

		LveCamera camera;
		camera.SetViewTarget(Vector3(-1.0f, -2.0f, 2.0f), Vector3(0.0f, 0.0f, 2.5f));

		LveGameObject viewerObject = LveGameObject::CreateGameObject();
		KeyboardMovementController cameraController{};

		std::chrono::time_point currentTime = std::chrono::high_resolution_clock::now();

		while (!m_window.ShouldClose())
		{
			glfwPollEvents();

			// Update time after polling because polling might block.
			std::chrono::time_point newTime = std::chrono::high_resolution_clock::now();
			F32 frameTime = std::chrono::duration<F32, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			cameraController.MoveInPlaneXZ(m_window.GetNativeWindow(), frameTime, viewerObject);
			camera.SetViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			F32 aspect = m_renderer.GetAspectRatio();
			camera.SetPerspectiveProjection(MathOp::Radians(50.f), aspect, 0.1f, 10.f);

			// Could be nullptr if, for example, the swapchain needs to be recreated.
			if (VkCommandBuffer commandBuffer = m_renderer.BeginFrame())
			{
				// Prepare frame info
				FrameInfo frameInfo{
					m_renderer.GetCurrentFrameIndex(),
					frameTime,
					commandBuffer,
					camera
				};

				// Update
				GlobalUbo ubo{};
				ubo.projectionView = camera.GetProjection() * camera.GetView();
				uboBuffers[frameInfo.frameIndex]->WriteToBuffer(&ubo);
				uboBuffers[frameInfo.frameIndex]->Flush();

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

				simpleRenderSystem.RenderGameObjects(frameInfo, m_gameObjects);

				m_renderer.EndSwapchainRenderPass(commandBuffer);
				m_renderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(m_device.GetDevice());
	}

	void FirstApp::LoadGameObjects()
	{
		// Cube
		// UniqueRef<LveModel> model = LveModel::CreateCubeModel(m_device, { 0.f, 0.f, 0.f });

		// Model
		UniqueRef<LveModel> smoothModel = LveModel::CreateModelFromFile(m_device, "models/smooth_vase.obj");
		UniqueRef<LveModel> flatModel = LveModel::CreateModelFromFile(m_device, "models/flat_vase.obj");

		LveGameObject gameObject = LveGameObject::CreateGameObject();
		gameObject.model = std::move(smoothModel);
		gameObject.transform.translation = { -0.5f, 0.0f, 3.0f };
		gameObject.transform.scale = Vector3(2.0f);

		LveGameObject gameObject2 = LveGameObject::CreateGameObject();
		gameObject2.model = std::move(flatModel);
		gameObject2.transform.translation = { 0.5f, 0.0f, 3.0f };
		gameObject2.transform.scale = Vector3(2.0f);

		m_gameObjects.push_back(std::move(gameObject));
		m_gameObjects.push_back(std::move(gameObject2));
	}

} // namespace lve
