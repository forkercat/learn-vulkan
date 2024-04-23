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
		Vector4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f }; // w is intensity
		Vector3 lightPosition{ -1.0f };
		alignas(16) Vector4 lightColor{ 1.0f }; // w is intensity
	};

	FirstApp::FirstApp()
	{
		m_globalDescriptorPool =
			LveDescriptorPool::Builder(m_device)
				// How many descriptor sets can be created from the pool.
				.SetMaxSets(LveSwapchain::MAX_FRAMES_IN_FLIGHT)
				// How many descriptors of this type are available in the pool.
				.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapchain::MAX_FRAMES_IN_FLIGHT)
				.Build();
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

		// Descriptors
		UniqueRef<LveDescriptorSetLayout> globalSetLayout =
			LveDescriptorSetLayout::Builder(m_device)
				.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.Build();

		std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapchain::MAX_FRAMES_IN_FLIGHT); // one set per frame
		for (int i = 0; i < globalDescriptorSets.size(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->DescriptorInfo();

			LveDescriptorWriter(*globalSetLayout, *m_globalDescriptorPool)
				.WriteBuffer(0, &bufferInfo)
				.Build(globalDescriptorSets[i]);
		}

		// Render system, camera, and controller
		SimpleRenderSystem simpleRenderSystem(
			m_device, m_renderer.GetSwapchainRenderPass(), globalSetLayout->GetDescriptorSetLayout());
		RainbowSystem rainbowSystem(0.4f);

		LveCamera camera;
		camera.SetViewTarget(Vector3(-1.0f, -2.0f, 2.0f), Vector3(0.0f, 0.0f, 2.5f));

		LveGameObject viewerObject = LveGameObject::CreateGameObject();
		viewerObject.transform.translation.z = -2.5;
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
			camera.SetPerspectiveProjection(MathOp::Radians(50.f), aspect, 0.1f, 100.f);

			// Could be nullptr if, for example, the swapchain needs to be recreated.
			if (VkCommandBuffer commandBuffer = m_renderer.BeginFrame())
			{
				// Prepare frame info
				U32 frameIndex = m_renderer.GetCurrentFrameIndex();
				FrameInfo frameInfo{
					.frameIndex = frameIndex,
					.frameTime = frameTime,
					.commandBuffer = commandBuffer,
					.globalDescriptorSet = globalDescriptorSets[frameIndex],
					.camera = camera,
					.gameObjects = m_gameObjects
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

				simpleRenderSystem.RenderGameObjects(frameInfo);

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
		UniqueRef<LveModel> quadModel = LveModel::CreateModelFromFile(m_device, "models/quad.obj");

		LveGameObject gameObject = LveGameObject::CreateGameObject();
		gameObject.model = std::move(smoothModel);
		gameObject.transform.translation = { -0.5f, 0.5f, 0.0f };
		gameObject.transform.scale = Vector3(2.0f);

		LveGameObject gameObject2 = LveGameObject::CreateGameObject();
		gameObject2.model = std::move(flatModel);
		gameObject2.transform.translation = { 0.5f, 0.5f, 0.0f };
		gameObject2.transform.scale = Vector3(2.0f);

		LveGameObject gameObjectQuad = LveGameObject::CreateGameObject();
		gameObjectQuad.model = std::move(quadModel);
		gameObjectQuad.transform.translation = { 0.0f, 0.5f, 0.0f };
		gameObjectQuad.transform.scale = { 3.0f, 1.0f, 3.0f };

		m_gameObjects.emplace(gameObject.GetId(), std::move(gameObject));
		m_gameObjects.emplace(gameObject2.GetId(), std::move(gameObject2));
		m_gameObjects.emplace(gameObjectQuad.GetId(), std::move(gameObjectQuad));
	}

} // namespace lve
